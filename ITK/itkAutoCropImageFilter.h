#ifndef __itkAutoCropImageFilter_h
#define __itkAutoCropImageFilter_h

#include <itkExtractImageFilter.h>
#include <itkImageToImageFilter.h>

namespace itk
{
/** \class AutoCropImageFilter
 *
 * \brief Crop an image to just contain the portion of the image that contains non-background pixels.
 */
template< class TInputImage, class TOutputImage >
class ITK_EXPORT AutoCropImageFilter :
  public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef AutoCropImageFilter                             Self;
  typedef ImageToImageFilter< TInputImage, TOutputImage > Superclass;
  typedef SmartPointer< Self >                            Pointer;
  typedef SmartPointer< const Self >                      ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(AutoCropImageFilter, ImageToImageFilter);

  /** Pixel types. */
  typedef TInputImage                       InputImageType;
  typedef typename TInputImage::Pointer     InputImagePointerType;
  typedef typename TInputImage::PixelType   InputImagePixelType;
  typedef typename TInputImage::IndexType   InputImageIndexType;
  typedef typename TInputImage::SizeType    InputImageSizeType;
  typedef typename TInputImage::RegionType  InputImageRegionType;

  typedef TOutputImage                      OutputImageType;
  typedef typename TOutputImage::Pointer    OutputImagePointerType;
  typedef typename TOutputImage::PixelType  OutputImagePixelType;
  typedef typename TOutputImage::IndexType  OutputImageIndexType;
  typedef typename TOutputImage::SizeType   OutputImageSizeType;
  typedef typename TOutputImage::RegionType OutputImageRegionType;

  /** Set/get the value considered to be the background. */
  itkSetMacro(BackgroundValue, InputImagePixelType);
  itkGetMacro(BackgroundValue, InputImagePixelType);

  /** Set/get the number of pixels by which the minimal bounding
   * region is padded.
   *
   * A non-zero pad size in any dimension makes the output image larger
   * in that dimension. Padding is added on both sides of the dimension.
   */
  itkSetMacro( PadRadius, InputImageSizeType );
  itkGetConstReferenceMacro( PadRadius, InputImageSizeType );

protected:
  AutoCropImageFilter();
  virtual ~AutoCropImageFilter();
  void PrintSelf(std::ostream & os, Indent indent) const;

  /** \todo Make faster multithreaded version */
  void GenerateData();

  /** The output of this filter has a different region than the input. */
  void GenerateOutputInformation();

private:
  InputImagePixelType m_BackgroundValue;

  InputImageSizeType  m_PadRadius;

  typedef itk::ExtractImageFilter< InputImageType, OutputImageType > ExtractType;
  typename ExtractType::Pointer m_ExtractFilter;

};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkAutoCropImageFilter.hxx"
#endif

#endif // __itkAutoCropImageFilter_h
