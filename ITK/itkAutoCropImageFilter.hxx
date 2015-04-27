#ifndef __itkAutoCropImageFilter_hxx
#define __itkAutoCropImageFilter_hxx

#include "itkAutoCropImageFilter.h"

#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkExtractImageFilter.h"

namespace itk
{

template< class TInputImage, class TOutputImage >
AutoCropImageFilter< TInputImage, TOutputImage >
::AutoCropImageFilter()
{
  m_BackgroundValue = NumericTraits< InputImagePixelType >::Zero;
  m_PadRadius.Fill( 0 );

  m_ExtractFilter = ExtractType::New();
}

template< class TInputImage, class TOutputImage >
AutoCropImageFilter< TInputImage, TOutputImage >
::~AutoCropImageFilter()
{
}

template< class TInputImage, class TOutputImage >
void
AutoCropImageFilter< TInputImage, TOutputImage >
::GenerateData()
{
  // Extract this region from the image
  m_ExtractFilter->SetInput( this->GetInput() );

  m_ExtractFilter->GraftOutput( this->GetOutput());
  m_ExtractFilter->Update();
  this->GraftOutput( m_ExtractFilter->GetOutput() );
}

template< class TInputImage, class TOutputImage >
void
AutoCropImageFilter< TInputImage, TOutputImage >
::GenerateOutputInformation()
{
  typename Superclass::OutputImagePointer outputPtr = this->GetOutput();
  typename Superclass::InputImageConstPointer inputPtr  = this->GetInput();

  if ( !outputPtr || !inputPtr )
    {
    return;
    }

  Superclass::GenerateOutputInformation();

  // Find the region containing foreground pixels
  InputImageRegionType largestRegion = this->GetInput()->GetLargestPossibleRegion();
  InputImageIndexType foregroundIndex = largestRegion.GetUpperIndex();
  InputImageIndexType foregroundUpperIndex = largestRegion.GetIndex();

  ImageRegionConstIteratorWithIndex< InputImageType > iterator( this->GetInput(), largestRegion );
  for ( iterator.GoToBegin(); !iterator.IsAtEnd(); ++iterator ) {
    if ( iterator.Get() != m_BackgroundValue )
    {
      InputImageIndexType index = iterator.GetIndex();
      for (int i = 0; i < TInputImage::ImageDimension; ++i) {
        foregroundIndex[i] = std::min( foregroundIndex[i], index[i] );
        foregroundUpperIndex[i] = std::max( foregroundUpperIndex[i], index[i] );
      }
    }
  }

  InputImageRegionType extractionRegion;
  extractionRegion.SetIndex( foregroundIndex );
  extractionRegion.SetUpperIndex( foregroundUpperIndex );

  // Pad the region
  extractionRegion.PadByRadius( m_PadRadius );

  // Crop the extraction region
  extractionRegion.Crop( largestRegion );

  m_ExtractFilter->SetExtractionRegion( extractionRegion );

  outputPtr->SetLargestPossibleRegion( extractionRegion );
}

template< class TInputImage, class TOutputImage >
void
AutoCropImageFilter< TInputImage, TOutputImage >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "BackgroundValue: " << m_BackgroundValue << std::endl;
  os << indent << "PadRadius: " << m_PadRadius << std::endl;
}

} // end namespace itk

#endif // __itkAutoCropImageFilter_hxx
