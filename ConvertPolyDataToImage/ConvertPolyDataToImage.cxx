#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "ConvertPolyDataToImageCLP.h"

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkVTKImageToImageFilter.h>

#include <vtkImageStencilToImage.h>
#include <vtkPolyData.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkXMLPolyDataReader.h>

#include <cstdlib>

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

template< typename T >
int GetReferenceInfo( const char* fileName, int extent[6], double origin[3], double spacing[3], T )
{
  const unsigned int Dimension = 3;
  typedef T InputPixelType;
  typedef itk::Image<InputPixelType,  Dimension> InputImageType;
  typedef itk::ImageFileReader<InputImageType>   ReaderType;

  typename ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( fileName );
  try
    {
    reader->Update();
    }
  catch ( itk::ExceptionObject & except )
    {
    std::cerr << "Could not read file '" << fileName << "'\n";
    std::cerr << except << std::endl;
    return EXIT_FAILURE;
    }

  InputImageType* referenceImage = reader->GetOutput();
  typename InputImageType::RegionType  inputRegion  = referenceImage->GetLargestPossibleRegion();
  typename InputImageType::IndexType   inputIndex   = inputRegion.GetIndex();
  typename InputImageType::SizeType    inputSize    = inputRegion.GetSize();
  typename InputImageType::PointType   inputOrigin  = referenceImage->GetOrigin();
  typename InputImageType::SpacingType inputSpacing = referenceImage->GetSpacing();

  for ( int i = 0; i < 3; ++i )
    {
    extent[2*i + 0] = inputIndex[i];
    extent[2*i + 1] = inputSize[i] - 1;
    origin[i]       = inputOrigin[i];
    spacing[i]      = inputSpacing[i];
    }

    for ( int i = 0; i < 3; ++i )
    {
    if ( i == 0 )
      {
      std::cout << "Image bounds: ";
      }
    std::cout << "(" << origin[i] << ", "
              << (origin[i] + (inputSize[i]-1)*inputSpacing[i]) << "), ";
    }
  std::cout << "\n";

  return EXIT_SUCCESS;
}

} // end anonymous namespace


int main( int argc, char* argv[] )
{
  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType     pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  // Output image info
  int    extent[6];
  double origin[3];
  double spacing[3];

  int result = EXIT_SUCCESS;

  try
    {
    GetImageType( referenceImage, pixelType, componentType );

    switch ( componentType )
      {
      case itk::ImageIOBase::UCHAR:
        result = GetReferenceInfo( referenceImage.c_str(), extent, origin, spacing,
                                   static_cast<unsigned char>(0) );
        break;
      case itk::ImageIOBase::CHAR:
        result = GetReferenceInfo( referenceImage.c_str(), extent, origin, spacing,
                                   static_cast<char>(0) );
        break;
      case itk::ImageIOBase::USHORT:
        result = GetReferenceInfo( referenceImage.c_str(), extent, origin, spacing,
                                   static_cast<unsigned short>(0) );
        break;
      case itk::ImageIOBase::SHORT:
        result = GetReferenceInfo( referenceImage.c_str(), extent, origin, spacing,
                                   static_cast<short>(0) );
        break;
      case itk::ImageIOBase::UINT:
        result = GetReferenceInfo( referenceImage.c_str(), extent, origin, spacing,
                                   static_cast<unsigned int>(0) );
        break;
      case itk::ImageIOBase::INT:
        result = GetReferenceInfo( referenceImage.c_str(), extent, origin, spacing,
                                   static_cast<int>(0) );
        break;
      case itk::ImageIOBase::ULONG:
        result = GetReferenceInfo( referenceImage.c_str(), extent, origin, spacing,
                                   static_cast<unsigned long>(0) );
        break;
      case itk::ImageIOBase::LONG:
        result = GetReferenceInfo( referenceImage.c_str(), extent, origin, spacing,
                                   static_cast<long>(0) );
        break;
      case itk::ImageIOBase::FLOAT:
        result = GetReferenceInfo( referenceImage.c_str(), extent, origin, spacing,
                                   static_cast<float>(0) );
        break;
      case itk::ImageIOBase::DOUBLE:
        result = GetReferenceInfo( referenceImage.c_str(), extent, origin, spacing,
                                   static_cast<double>(0) );
        break;
      default:
        std::cout << "unknown component type" << std::endl;
        break;
      }

    }
  catch ( itk::ExceptionObject & except )
    {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << except << std::endl;
    return EXIT_FAILURE;
    }

  if ( result != EXIT_SUCCESS )
    {
    std::cerr << "Bad result when getting reference image info\n";
    return result;
    }

  // Read model
  vtkSmartPointer<vtkXMLPolyDataReader> modelReader =
    vtkSmartPointer<vtkXMLPolyDataReader>::New();
  modelReader->SetFileName( inputModel.c_str() );

  // Slicer assumes poly data is in RAS space. We are operating in
  // LPS, so we need to convert the surface here.
  vtkSmartPointer<vtkTransform> RASToLPSTransform  =
    vtkSmartPointer<vtkTransform>::New();
  RASToLPSTransform->Scale( -1.0, -1.0, 1.0 );

  vtkSmartPointer<vtkTransformFilter> transformedModel =
    vtkSmartPointer<vtkTransformFilter>::New();
  transformedModel->SetTransform( RASToLPSTransform );
  transformedModel->SetInputConnection( modelReader->GetOutputPort() );
  transformedModel->Update();

  double bounds[6];
  transformedModel->GetOutput()->GetBounds(bounds);
  std::cout << "Model bounds: "
            << "(" << bounds[0] << ", " << bounds[1] << "), "
            << "(" << bounds[2] << ", " << bounds[3] << "), "
            << "(" << bounds[4] << ", " << bounds[5] << ")" << std::endl;

  // Now that we have the reference image info, convert the model to
  // an image of that extent.
  vtkSmartPointer< vtkPolyDataToImageStencil > stencilSource =
    vtkSmartPointer< vtkPolyDataToImageStencil >::New();
  stencilSource->SetOutputOrigin( origin );
  stencilSource->SetOutputSpacing( spacing );
  stencilSource->SetOutputWholeExtent( extent );
  stencilSource->SetInputConnection( transformedModel->GetOutputPort() );

  vtkSmartPointer<vtkImageStencilToImage> stencil =
    vtkSmartPointer<vtkImageStencilToImage>::New();
  stencil->SetInputConnection( stencilSource->GetOutputPort() );
  stencil->SetOutputScalarTypeToUnsignedChar();
  stencil->SetInsideValue( interiorValue );
  stencil->SetOutsideValue( exteriorValue );
  stencil->Update();

  // Write the output
  typedef itk::Image< unsigned char, 3 >                OutputImageType;
  typedef itk::VTKImageToImageFilter< OutputImageType > VTK2ITKType;
  typedef itk::ImageFileWriter< OutputImageType >       WriterType;

  VTK2ITKType::Pointer vtk2itk = VTK2ITKType::New();
  vtk2itk->SetInput( stencil->GetOutput() );

  WriterType::Pointer writer = WriterType::New();
  writer->SetInput( vtk2itk->GetOutput() );
  writer->SetFileName( outputImage.c_str() );
  try
    {
    writer->Update();
    }
  catch ( itk::ExceptionObject & except )
    {
    std::cerr << "Could not write file '" << outputImage << "'\n";
    std::cerr << except << "\n";
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
