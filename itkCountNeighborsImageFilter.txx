/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkCountNeighborsImageFilter.txx,v $
  Language:  C++
  Date:      $Date: 2008-10-17 20:50:03 $
  Version:   $Revision: 1.11 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkCountNeighborsImageFilter_txx
#define __itkCountNeighborsImageFilter_txx
#include "itkCountNeighborsImageFilter.h"

#include "itkConstNeighborhoodIterator.h"
#include "itkNeighborhoodInnerProduct.h"
#include "itkImageRegionIterator.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkConstantBoundaryCondition.h"
#include "itkOffset.h"
#include "itkProgressReporter.h"

#include <vector>
#include <algorithm>

namespace itk
{

template <class TInputImage, class TOutputImage>
CountNeighborsImageFilter<TInputImage, TOutputImage>
::CountNeighborsImageFilter()
{
  m_Radius.Fill(1);
  m_AtValue = NumericTraits<InputPixelType>::max();
  m_LowerValue = NumericTraits<InputPixelType>::Zero;
  m_UpperValue = NumericTraits<InputPixelType>::max();
  m_PixelValueOfInterest = NumericTraits<InputPixelType>::Zero;

  m_PixelValueFlag = false;  
  m_SelectedStrategy = NONZERO;
  m_Indices.clear();
}

template <class TInputImage, class TOutputImage>
void 
CountNeighborsImageFilter<TInputImage, TOutputImage>
::GenerateInputRequestedRegion() throw (InvalidRequestedRegionError)
{
  // call the superclass' implementation of this method
  Superclass::GenerateInputRequestedRegion();
  
  // get pointers to the input and output
  typename Superclass::InputImagePointer inputPtr = 
    const_cast< TInputImage * >( this->GetInput() );
  typename Superclass::OutputImagePointer outputPtr = this->GetOutput();
  
  if ( !inputPtr || !outputPtr )
    {
    return;
    }

  // get a copy of the input requested region (should equal the output
  // requested region)
  typename TInputImage::RegionType inputRequestedRegion;
  inputRequestedRegion = inputPtr->GetRequestedRegion();

  // pad the input requested region by the operator radius
  inputRequestedRegion.PadByRadius( m_Radius );

  // crop the input requested region at the input's largest possible region
  if ( inputRequestedRegion.Crop(inputPtr->GetLargestPossibleRegion()) )
    {
    inputPtr->SetRequestedRegion( inputRequestedRegion );
    return;
    }
  else
    {
    // Couldn't crop the region (requested region is outside the largest
    // possible region).  Throw an exception.

    // store what we tried to request (prior to trying to crop)
    inputPtr->SetRequestedRegion( inputRequestedRegion );
    
    // build an exception
    InvalidRequestedRegionError e(__FILE__, __LINE__);
    e.SetLocation(ITK_LOCATION);
    e.SetDescription("Requested region is (at least partially) outside the largest possible region.");
    e.SetDataObject(inputPtr);
    throw e;
    }
}


template< class TInputImage, class TOutputImage>
void
CountNeighborsImageFilter< TInputImage, TOutputImage>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
                       int threadId)
{
  
  ConstantBoundaryCondition<InputImageType> nbc;
  nbc.SetConstant(0);

  ConstNeighborhoodIterator<InputImageType> bit;
  ImageRegionIterator<OutputImageType> it;
  
  // Allocate output
  typename OutputImageType::Pointer output = this->GetOutput();
  typename InputImageType::ConstPointer input  = this->GetInput();
  
  // Find the data-set boundary "faces"
  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType faceList;
  NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType> bC;
  faceList = bC(input, outputRegionForThread, m_Radius);

  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType::iterator fit;

  ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());
  
  unsigned int count = 0;
  InputOffsetType centerOffset;
  centerOffset.Fill(0);

  // Process each of the boundary faces.  These are N-d regions which border
  // the edge of the buffer.
  for (fit = faceList.begin(); fit != faceList.end(); ++fit)
    { 
    bit = ConstNeighborhoodIterator<InputImageType>(m_Radius, input, *fit);
    it  = ImageRegionIterator<OutputImageType>(output, *fit);
    bit.OverrideBoundaryCondition(&nbc);
    bit.GoToBegin();
    
    unsigned int neighborhoodSize = bit.Size();
    
    while ( ! bit.IsAtEnd() )
      {
      const InputPixelType inpixel = bit.GetCenterPixel();
      count = 0;
      if(m_PixelValueFlag)
        {
        if(inpixel == m_PixelValueOfInterest)
          {
          // count the 'interesting' pixels in the neighborhood
          for(unsigned int i = 0; i < neighborhoodSize; ++i)
            {
            // Don't include center pixel
            if(bit.GetIndex(i) != bit.GetIndex(centerOffset))
              {
              InputPixelType value = bit.GetPixel(i);
              if(m_SelectedStrategy == ABOVE)
                {
                if(value >= m_LowerValue)
                  count++;
                }
              else if(m_SelectedStrategy == BELOW)
                {
                if(value <= m_UpperValue)
                  count++;
                }
              else if(m_SelectedStrategy == BETWEEN)
                {
                if(value >= m_LowerValue && value <= m_UpperValue)
                  count++;
                }
              else if(m_SelectedStrategy == AT)
                {
                if(value == m_AtValue)
                  count++;
                }
              else if(m_SelectedStrategy == NONZERO)
                {
                if(value != NumericTraits<InputPixelType>::Zero)
                  count++;
                }
              else
                count = 0;
              }
            }
          }
        }
      else
        {
        count = 0;
        // count the 'interesting' pixels in the neighborhood
        for(unsigned int i = 0; i < neighborhoodSize; ++i)
          {
          InputPixelType value = bit.GetPixel(i);
          if(m_SelectedStrategy == ABOVE)
            {
            if(value >= m_LowerValue)
              count++;
            }
          else if(m_SelectedStrategy == BELOW)
            {
            if(value <= m_UpperValue)
              count++;
            }
          else if(m_SelectedStrategy == BETWEEN)
            {
            if(value >= m_LowerValue && value <= m_UpperValue)
              count++;
            }
          else if(m_SelectedStrategy == AT)
            {
            if(value == m_AtValue)
              count++;
            }
          else if(m_SelectedStrategy == NONZERO)
            {
            if(value != NumericTraits<InputPixelType>::Zero)
              count++;
            }
          else
            count = 0;
          }
        }
      it.Set( static_cast<OutputPixelType>( count ) );

      ++bit;
      ++it;
      progress.CompletedPixel();
      }
    }
}

/**
 * Compute the indices of pixels for a given number of neighbors computed
 * Update() must be called after update!
 */
template <class TInputImage, class TOutput>
void
CountNeighborsImageFilter<TInputImage, TOutput>
::ComputeIndices(OutputPixelType numberOfNeighbors)
{
  typename OutputImageType::Pointer outputImage = this->GetOutput(); 
  if(!outputImage)
    {
    ExceptionObject err(__FILE__, __LINE__);
    err.SetLocation( ITK_LOCATION );
    err.SetDescription( "Output image is null. Call Update() first." );
    throw err;
    }
  
  typedef itk::ImageRegionIterator<OutputImageType> ImageIteratorType;
  ImageIteratorType it(outputImage, outputImage->GetLargestPossibleRegion());
  for(it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
    if(it.Value() == numberOfNeighbors)
      m_Indices.push_back(it.GetIndex());
    }
}
 
/**
 * Standard "PrintSelf" method
 */
template <class TInputImage, class TOutput>
void
CountNeighborsImageFilter<TInputImage, TOutput>
::PrintSelf(
  std::ostream& os, 
  Indent indent) const
{
  Superclass::PrintSelf( os, indent );
  os << indent << "Radius: " << m_Radius << std::endl;
  os << indent << "Lower Value   : " << static_cast<typename NumericTraits<InputPixelType>::PrintType>( m_LowerValue ) << std::endl;
  os << indent << "Upper Value   : " << static_cast<typename NumericTraits<InputPixelType>::PrintType>( m_UpperValue ) << std::endl;
  os << indent << "At Value   : " << static_cast<typename NumericTraits<InputPixelType>::PrintType>( m_AtValue ) << std::endl;
  os << indent << "Pixel Value Of Interest   : " << static_cast<typename NumericTraits<InputPixelType>::PrintType>( m_PixelValueOfInterest ) << std::endl;
  os << indent << "Pixel Value Flag   : " << m_PixelValueFlag << std::endl;
  os << indent << "Count Strategy   : " << m_SelectedStrategy << std::endl;
}

} // end namespace itk

#endif
