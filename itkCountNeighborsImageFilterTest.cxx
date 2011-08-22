#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <fstream>
#include "itkCountNeighborsImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkPNGImageIOFactory.h"
#include "itkTextOutput.h"
#include "itkFilterWatcher.h"

int main(int ac, char* av[])
{
  // Comment the following if you want to use the itk text output window
  itk::OutputWindow::SetInstance(itk::TextOutput::New());

  if(ac < 2)
    {
    std::cerr << "Usage: " << av[0] << " InputImage \n";
    return -1;
    }

  // Image typedef
  typedef itk::Image<unsigned char, 2> ImageType;
  
  // Read image from disk
  itk::ImageFileReader<ImageType>::Pointer imageReader = itk::ImageFileReader<ImageType>::New();
  imageReader->SetFileName(av[1]);
  imageReader->Update();
  
  // Set input image
  ImageType::Pointer inputImage = ImageType::New();
  inputImage = imageReader->GetOutput();
  inputImage->DisconnectPipeline();
  
  // Create the filter
  typedef itk::CountNeighborsImageFilter<ImageType, ImageType> FilterType;
  FilterType::Pointer filter = FilterType::New();
  FilterWatcher filterWatch(filter);
  filter->SetInput(inputImage);
  
  
  // Test radius of 1, CountNonZero, and a pixel value of 255
  // Set filter parameters
  ImageType::SizeType radius;
  radius.Fill(1);
  filter->SetRadius(radius);
  filter->SetCountNonZero();
  filter->SetValueOfInterest(255);

  // Execute filter
  try
    {
    filter->Update();
    }
  catch (itk::ExceptionObject& e)
    {
    std::cerr << "Exception detected: "  << e.GetDescription();
    return -1;
    }

  // Write output image to disk
  itk::ImageFileWriter<ImageType>::Pointer writer = itk::ImageFileWriter<ImageType>::New();
  writer->SetInput(filter->GetOutput());
  writer->SetFileName("output1.png");
  writer->Update();
  
  // Test radius of 1, CountAboveValue, and for any pixel value
  // Set filter parameters
  radius.Fill(1);
  filter->SetRadius(radius);
  filter->SetCountAboveValue(254);
  filter->SetPixelValueFlag(0);

  // Execute filter
  try
    {
    filter->Update();
    }
  catch (itk::ExceptionObject& e)
    {
    std::cerr << "Exception detected: "  << e.GetDescription();
    return -1;
    }

  // Write output image to disk
  writer->SetInput(filter->GetOutput());
  writer->SetFileName("output2.png");
  writer->Update();
  
  // Test radius of 2, CountBetweenValue, and for any pixel value
  // Set filter parameters
  radius.Fill(2);
  filter->SetRadius(radius);
  filter->SetCountBetweenValues(126, 196);
  filter->SetPixelValueFlag(0);

  // Execute filter
  try
    {
    filter->Update();
    }
  catch (itk::ExceptionObject& e)
    {
    std::cerr << "Exception detected: "  << e.GetDescription();
    return -1;
    }

  // Write output image to disk
  writer->SetInput(filter->GetOutput());
  writer->SetFileName("output3.png");
  writer->Update();
  
  // Test radius of 1, CountAt, and for any pixel value
  // Set filter parameters
  radius.Fill(1);
  filter->SetRadius(radius);
  filter->SetCountAtValue(255);
  filter->SetValueOfInterest(127);

  // Execute filter
  try
    {
    filter->Update();
    }
  catch (itk::ExceptionObject& e)
    {
    std::cerr << "Exception detected: "  << e.GetDescription();
    return -1;
    }

  // Write output image to disk
  writer->SetInput(filter->GetOutput());
  writer->SetFileName("output4.png");
  writer->Update();
  
  return EXIT_SUCCESS;
}
