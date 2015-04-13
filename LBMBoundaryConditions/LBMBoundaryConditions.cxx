#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "LBMNoseSphere.h"
#include "LBMBoundaryConditionsCLP.h"

#include <itkConstantPadImageFilter.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

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

/*******************************************************************/
/** Do all the work. */
/*******************************************************************/
template< typename T >
int DoIt(int argc, char* argv[], T)
{
  PARSE_ARGS;

  const unsigned int Dimension = 3;
  typedef T InputPixelType;
  typedef itk::Image< InputPixelType, Dimension > InputImageType;
  typedef itk::Image< char, Dimension >           LabelImageType;
  typedef itk::ImageFileReader< InputImageType >  InputReaderType;
  typedef itk::ImageFileReader< LabelImageType >  LabelReaderType;

  typename InputReaderType::Pointer inputReader = InputReaderType::New();
  inputReader->SetFileName( ctImage.c_str() );
  try
    {
    inputReader->Update();
    }
  catch ( itk::ExceptionObject & except )
    {
    std::cerr << "Could not read CT file '" << ctImage << "'\n";
    std::cerr << except << std::endl;
    return EXIT_FAILURE;
    }

  typename LabelReaderType::Pointer labelReader = LabelReaderType::New();
  labelReader->SetFileName( segmentationImage.c_str() );
  try
    {
    labelReader->Update();
    }
  catch ( itk::ExceptionObject & except )
    {
    std::cerr << "Could not read segmentation file '" << segmentationImage << "'\n";
    std::cout << except << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "label image: " << labelReader->GetOutput()->GetLargestPossibleRegion() << std::endl;

#define IMPOSED_INFLOW 100
#define IMPOSED_OUTFLOW 200
  
  const int INTERIOR      =   0;
  const int EXTERIOR      =  -1;
  const int INFLOW_LOCAL  = IMPOSED_INFLOW;
  const int OUTFLOW_LOCAL = IMPOSED_OUTFLOW;

  // Add the nose sphere to the segmentation
  double sphereCenter[3];
  sphereCenter[0] = noseSphereCenter[0];
  sphereCenter[1] = noseSphereCenter[1];
  sphereCenter[2] = noseSphereCenter[2];

  double sphereRadius = noseSphereRadius;

  // Pad the image to fully contain the nose sphere
  LabelImageType::RegionType sphereRegion =
    GetNoseSphereRegion( sphereCenter,
                         sphereRadius,
                         labelReader->GetOutput() );

  // Expand image if the nose sphere falls outside of it.
  LabelImageType::SizeType lowerBound;
  lowerBound.Fill( 0 );
  LabelImageType::SizeType upperBound;
  upperBound.Fill( 0 );
  LabelImageType::RegionType imageRegion =
    labelReader->GetOutput()->GetLargestPossibleRegion();
  LabelImageType::IndexType imageIndex = imageRegion.GetIndex();
  LabelImageType::IndexType imageUpperIndex = imageRegion.GetUpperIndex();

  LabelImageType::IndexType sphereIndex = sphereRegion.GetIndex();
  LabelImageType::IndexType sphereUpperIndex = sphereRegion.GetUpperIndex();
  for ( unsigned int i = 0; i < LabelImageType::ImageDimension; ++i ) {
    if ( sphereIndex[i] < imageIndex[i] ) {
      lowerBound[i] = imageIndex[i] - sphereIndex[i];
    }
    if ( sphereUpperIndex[i] > imageUpperIndex[i] ) {
      upperBound[i] = sphereUpperIndex[i] - imageUpperIndex[i];
    }
  }

  typedef itk::ConstantPadImageFilter< InputImageType, InputImageType >
    SourcePadFilterType;
  SourcePadFilterType::Pointer sourcePadFilter = SourcePadFilterType::New();
  sourcePadFilter->SetPadLowerBound( lowerBound );
  sourcePadFilter->SetPadUpperBound( upperBound );
  sourcePadFilter->SetConstant( -1024 );
  sourcePadFilter->SetInput( inputReader->GetOutput() );

  typedef itk::ConstantPadImageFilter< LabelImageType, LabelImageType >
    BinaryPadFilterType;
  BinaryPadFilterType::Pointer binaryPadFilter = BinaryPadFilterType::New();
  binaryPadFilter->SetPadLowerBound( lowerBound );
  binaryPadFilter->SetPadUpperBound( upperBound );
  binaryPadFilter->SetConstant( EXTERIOR );
  binaryPadFilter->SetInput( labelReader->GetOutput() );

  LabelImageType* binaryImage = binaryPadFilter->GetOutput();

  AddNoseSphere( sphereCenter, sphereRadius, binaryImage,
                 sourcePadFilter->GetOutput(), segmentationThreshold,
                 INTERIOR, EXTERIOR, INFLOW_LOCAL );

  // MORE STUFF HERE

  // Write the output
  typedef itk::ImageFileWriter< LabelImageType > OutputWriter;
  typename OutputWriter::Pointer writer = OutputWriter::New();
  writer->SetFileName( lbmImage.c_str() );
  writer->SetInput( binaryImage );
  try
    {
    writer->Update();
    }
  catch ( itk::ExceptionObject & except )
    {
    std::cerr << "Could not write file '" << lbmImage << "'\n";
    std::cerr << except << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

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
    GetImageType( ctImage, pixelType, componentType );

    switch ( pixelType )
      {
      case itk::ImageIOBase::CHAR:
        result = DoIt( argc, argv, static_cast<char>(0) );
        break;
      case itk::ImageIOBase::UCHAR:
        result = DoIt( argc, argv, static_cast<unsigned char>(0) );
        break;
      case itk::ImageIOBase::SHORT:
        result = DoIt( argc, argv, static_cast<short>(0) );
        break;
      case itk::ImageIOBase::USHORT:
        result = DoIt( argc, argv, static_cast<unsigned short>(0) );
        break;
      case itk::ImageIOBase::INT:
        result = DoIt( argc, argv, static_cast<int>(0) );
        break;
      case itk::ImageIOBase::UINT:
        result = DoIt( argc, argv, static_cast<unsigned int>(0) );
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
