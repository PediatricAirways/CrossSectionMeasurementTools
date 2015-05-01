#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "ResampleImageCLP.h"

#include <itkBSplineInterpolateImageFunction.h>
#include <itkIdentityTransform.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkResampleImageFilter.h>

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

/*******************************************************************/
/** Query the image type. */
/*******************************************************************/
void GetImageType ( std::string fileName,
                    itk::ImageIOBase::IOPixelType & pixelType,
                    itk::ImageIOBase::IOComponentType & componentType)
{
  typedef itk::Image<unsigned char, 3> ImageType;
  itk::ImageFileReader<ImageType>::Pointer imageReader =
    itk::ImageFileReader<ImageType>::New();

  imageReader->SetFileName(fileName.c_str());
  imageReader->UpdateOutputInformation();

  pixelType = imageReader->GetImageIO()->GetPixelType();
  componentType = imageReader->GetImageIO()->GetComponentType();
}

} // end anonymous namespace

/*******************************************************************/
/** Do all the work. */
/*******************************************************************/
template< typename T >
int DoIt(int argc, char* argv[], T)
{
  PARSE_ARGS;

  const unsigned int Dimension = 3;
  typedef T PixelType;
  typedef itk::Image< PixelType, Dimension > ImageType;

  typedef itk::ImageFileReader< ImageType > ReaderType;
  typename ReaderType::Pointer inputReader = ReaderType::New();
  inputReader->SetFileName( inputImage.c_str() );
  try
    {
    inputReader->Update();
    }
  catch ( itk::ExceptionObject & except )
    {
    std::cerr << "Could not read input file '" << inputImage << "'\n";
    std::cerr << except << std::endl;
    return EXIT_FAILURE;
    }

  typename ImageType::Pointer input = inputReader->GetOutput();

  typedef double InterpolatorPrecision;
  typedef itk::ResampleImageFilter< ImageType, ImageType, InterpolatorPrecision >
                                                                            ResampleFilterType;
  typedef itk::IdentityTransform< double, ImageType::ImageDimension >       TransformType;

  typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  typename TransformType::Pointer transform = TransformType::New();
  resampler->SetTransform( transform );
  if ( interpolator == "Nearest" )
    {
    typedef itk::NearestNeighborInterpolateImageFunction< ImageType, InterpolatorPrecision >
      InterpolatorType;
    typename InterpolatorType::Pointer interpolatorFunction = InterpolatorType::New();
    resampler->SetInterpolator( interpolatorFunction );
    }
  else if ( interpolator == "Linear" )
    {
    typedef itk::LinearInterpolateImageFunction< ImageType, InterpolatorPrecision >
      InterpolatorType;
    typename InterpolatorType::Pointer interpolatorFunction = InterpolatorType::New();
    resampler->SetInterpolator( interpolatorFunction );
    }
  else if ( interpolator == "BSpline" )
    {
    typedef itk::BSplineInterpolateImageFunction< ImageType, double, double > InterpolatorType;
    typename InterpolatorType::Pointer interpolatorFunction = InterpolatorType::New();
    interpolatorFunction->SetSplineOrder( 3 );
    resampler->SetInterpolator( interpolatorFunction );
    }
  else
    {
    std::cerr << "Unknown interpolator '" << interpolator << "'\n";
    return EXIT_FAILURE;
    }

  typename ImageType::SpacingType resampleSpacing;
  resampleSpacing[0] = spacing[0];
  resampleSpacing[1] = spacing[1];
  resampleSpacing[2] = spacing[2];

  // Set most of the output settings from the input image
  resampler->SetOutputParametersFromImage( input );

  // Set spacing
  resampler->SetOutputSpacing( resampleSpacing );

  typename ImageType::SpacingType inputSpacing = input->GetSpacing();
  typename ImageType::RegionType  inputRegion = input->GetLargestPossibleRegion();
  typename ImageType::SizeType    inputSize = inputRegion.GetSize();

  typename ImageType::SizeType resampleSize;
  for ( int i = 0; i < 3; ++i )
    {
    double originalSize = (inputSize[i] - 1) * inputSpacing[i];
    resampleSize[i] = originalSize / spacing[i];
    }

  resampler->SetSize( resampleSize );
  resampler->SetInput( inputReader->GetOutput() );

  typedef itk::ImageFileWriter< ImageType > WriterType;
  typename WriterType::Pointer outputWriter = WriterType::New();
  outputWriter->SetFileName( outputImage.c_str() );
  outputWriter->SetInput( resampler->GetOutput() );
  try
    {
    outputWriter->Update();
    }
  catch ( itk::ExceptionObject & except )
    {
    std::cerr << "Could not write output file '" << outputImage << "'\n";
    std::cerr << except << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

/*******************************************************************/
int main( int argc, char* argv[] )
{
  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType     pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  int result = EXIT_SUCCESS;

  try
    {
    GetImageType( inputImage, pixelType, componentType );

    switch ( componentType )
      {
      case itk::ImageIOBase::UCHAR:
        result = DoIt( argc, argv, static_cast<unsigned char>(0) );
        break;
      case itk::ImageIOBase::CHAR:
        result = DoIt( argc, argv, static_cast<char>(0) );
        break;
      case itk::ImageIOBase::USHORT:
        result = DoIt( argc, argv, static_cast<unsigned short>(0) );
        break;
      case itk::ImageIOBase::SHORT:
        result = DoIt( argc, argv, static_cast<short>(0) );
        break;
      case itk::ImageIOBase::UINT:
        result = DoIt( argc, argv, static_cast<unsigned int>(0) );
        break;
      case itk::ImageIOBase::INT:
        result = DoIt( argc, argv, static_cast<int>(0) );
        break;
      case itk::ImageIOBase::FLOAT:
        result = DoIt( argc, argv, static_cast<float>(0) );
        break;
      case itk::ImageIOBase::DOUBLE:
        result = DoIt( argc, argv, static_cast<double>(0) );
        break;
      default:
        std::cerr << "Unknown pixel type " << pixelType << std::endl;
        result = EXIT_FAILURE;
        break;
      }
    }
  catch ( itk::ExceptionObject & except )
    {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << except << std::endl;
    return EXIT_FAILURE;
    }

  return result;
}
