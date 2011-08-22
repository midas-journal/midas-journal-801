#ifndef __itkCountNeighborsImageFilter_h
#define __itkCountNeighborsImageFilter_h

#include "itkImageToImageFilter.h"
#include "itkImage.h"

namespace itk
{
/** \class CountNeighborsImageFilter
 * \brief Visit each pixel in an image, count the number of neighboring
 * pixels, and assign the count to the pixel.
 *
 * m_Radius determines the size of the neighborhood to sample at each pixel.
 * A value of 1 only samples adjacent pixels (default). 
 * 
 * Multiple options are available for including pixels in the count:
 * - Non-zero (default)
 * - Below a value
 * - Above a value
 * - Between two value
 * - Equal to a value
 *
 * Neighbor counting can be restricted to pixels of a specific value
 * by SetValueOfInterest() 
 *
 * \sa Image
 * \sa Neighborhood
 * \sa NeighborhoodOperator
 * \sa NeighborhoodIterator
 * 
 * \ingroup IntensityImageFilters
 */
template <class TInputImage, class TOutputImage>
class ITK_EXPORT CountNeighborsImageFilter :
    public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Extract dimension from input and output image. */
  itkStaticConstMacro(InputImageDimension, unsigned int,
                      TInputImage::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int,
                      TOutputImage::ImageDimension);

  /** Convenient typedefs for simplifying declarations. */
  typedef TInputImage  InputImageType;
  typedef TOutputImage OutputImageType;

  /** Standard class typedefs. */
  typedef CountNeighborsImageFilter                            Self;
  typedef ImageToImageFilter< InputImageType, OutputImageType> Superclass;
  typedef SmartPointer<Self>                                   Pointer;
  typedef SmartPointer<const Self>                             ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);
  
  /** Run-time type information (and related methods). */
  itkTypeMacro(CountNeighborsImageFilter, ImageToImageFilter);
  
  /** Image typedef support. */
  typedef typename InputImageType::PixelType  InputPixelType;
  typedef typename OutputImageType::PixelType OutputPixelType;

  typedef typename InputImageType::RegionType  InputImageRegionType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;

  typedef typename InputImageType::SizeType InputSizeType;
  typedef typename InputImageType::OffsetType InputOffsetType;
  
  typedef typename OutputImageType::IndexType OutputIndexType;

  /** Counting strategies. */
  enum CountStrategy{ABOVE, BELOW, BETWEEN, AT, NONZERO};
  
  /** Set the radius of the neighborhood. */
  itkSetMacro(Radius, InputSizeType);

  /** Get the radius of the neighborhood. */
  itkGetConstReferenceMacro(Radius, InputSizeType);
  
  /**
  * Set value to count above. */
  void SetCountAboveValue(InputPixelType aboveValue)
    {
	m_SelectedStrategy = ABOVE;
	m_LowerValue = aboveValue;
	}
	
  /** Set/Get flag to determine count criteria. */
  itkSetMacro(SelectedStrategy, CountStrategy);
  itkGetConstReferenceMacro(SelectedStrategy, CountStrategy);

  /**
  * Set value to count below. */
  void SetCountBelowValue(InputPixelType belowValue)
    {
	m_SelectedStrategy = BELOW;
	m_UpperValue = belowValue;
    }

  /**
  * Set values to count between. */
  void SetCountBetweenValues(InputPixelType lowerValue, InputPixelType upperValue)
    {
	m_SelectedStrategy = BETWEEN;
	m_LowerValue = lowerValue;
	m_UpperValue = upperValue;
    }
  
  /**
  * Set value to count at. */
  void SetCountAtValue(InputPixelType atValue)
    {
	m_SelectedStrategy = AT;
	m_AtValue = atValue;
	}
  
  /**
  * Set counting of all nonzero pixels. */
  void SetCountNonZero(void)
    {
	m_SelectedStrategy = NONZERO;
	}
	
  /**
  * Restrict neighbor count to pixels of this value. */
  void SetValueOfInterest(InputPixelType value)
    {
	m_PixelValueFlag = true;
    m_PixelValueOfInterest = value;
	}
  
  /**
  * Set flag to enable/disable counting in neighborhoods centered at a specific pixel value. */
  itkSetMacro(PixelValueFlag, bool);
	
  /**
  * Get functions for value variables. */
  itkGetConstReferenceMacro(LowerValue, InputPixelType);
  itkGetConstReferenceMacro(UpperValue, InputPixelType);
  itkGetConstReferenceMacro(AtValue, InputPixelType);  
  itkGetConstReferenceMacro(PixelValueOfInterest, InputPixelType);  
  
  /**
  * Compute the indices of pixels for a given number of neighbors computed
  * Update() must be called first! */
   void ComputeIndices(OutputPixelType numberOfNeighbors);

  /** Get indices. */
  std::vector<OutputIndexType> GetIndices(void) {return m_Indices;}
    
  /** CountNeighborsImageFilter needs a larger input requested region than
   * the output requested region.  As such, CountNeighborsImageFilter needs
   * to provide an implementation for GenerateInputRequestedRegion()
   * in order to inform the pipeline execution model.
   *
   * \sa ImageToImageFilter::GenerateInputRequestedRegion() */
  virtual void GenerateInputRequestedRegion() throw(InvalidRequestedRegionError);

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro(InputEqualityComparableCheck,
    (Concept::EqualityComparable<InputPixelType>));
  itkConceptMacro(IntConvertibleToInputCheck,
    (Concept::Convertible<int, InputPixelType>));
  itkConceptMacro(InputConvertibleToOutputCheck,
    (Concept::Convertible<InputPixelType, OutputPixelType>));
  itkConceptMacro(SameDimensionCheck,
    (Concept::SameDimension<InputImageDimension, OutputImageDimension>));
  itkConceptMacro(InputOStreamWritableCheck,
    (Concept::OStreamWritable<InputPixelType>));
  /** End concept checking */
#endif

protected:
  CountNeighborsImageFilter();
  virtual ~CountNeighborsImageFilter() {}
  void PrintSelf(std::ostream& os, Indent indent) const;

  /** CountNeighborsImageFilter can be implemented as a multithreaded filter.
   * Therefore, this implementation provides a ThreadedGenerateData()
   * routine which is called for each processing thread. The output
   * image data is allocated automatically by the superclass prior to
   * calling ThreadedGenerateData().  ThreadedGenerateData can only
   * write to the portion of the output image specified by the
   * parameter "outputRegionForThread"
   *
   * \sa ImageToImageFilter::ThreadedGenerateData(),
   *     ImageToImageFilter::GenerateData() */
  void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
                            int threadId );

private:
  CountNeighborsImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  InputSizeType m_Radius;
  InputPixelType     m_LowerValue;
  InputPixelType     m_UpperValue;
  InputPixelType     m_AtValue;
  InputPixelType     m_PixelValueOfInterest;
  CountStrategy      m_SelectedStrategy;
  bool               m_PixelValueFlag;
  std::vector<OutputIndexType> m_Indices;
  
};
  
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkCountNeighborsImageFilter.txx"
#endif

#endif
