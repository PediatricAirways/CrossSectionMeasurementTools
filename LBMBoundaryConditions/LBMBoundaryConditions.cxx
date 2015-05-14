#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "LBMNoseSphere.h"
#include "LBMBoundaryConditionsCLP.h"

#include <itkAutoCropImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkConstantPadImageFilter.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkRelabelComponentImageFilter.h>
#include <itkVTKImageIO.h>

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
  typedef itk::Image< short, Dimension >          LabelImageType;
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

#define IMPOSED_INFLOW 100
#define IMPOSED_OUTFLOW 200

  const int INTERIOR      =   0;
  const int EXTERIOR      =  -1;
  const int INFLOW_LOCAL  = IMPOSED_INFLOW;
  const int OUTFLOW_LOCAL = IMPOSED_OUTFLOW;

  // Remove small islands from the image
  // WARNING: input to the ConnectedComponentImageFilter requires a
  // non-negative background value, otherwise it will likely crash.
  typedef itk::ConnectedComponentImageFilter< LabelImageType,
                                              LabelImageType,
                                              LabelImageType >
    ConnectedComponentFilterType;
  ConnectedComponentFilterType::Pointer connectedComponentFilter =
    ConnectedComponentFilterType::New();
  connectedComponentFilter->SetBackgroundValue( 0 );
  connectedComponentFilter->FullyConnectedOn();
  connectedComponentFilter->SetInput( labelReader->GetOutput() );

  typedef itk::RelabelComponentImageFilter< LabelImageType,
                                            LabelImageType >
    RelabelComponentFilterType;
  RelabelComponentFilterType::Pointer relabelComponentFilter =
    RelabelComponentFilterType::New();
  relabelComponentFilter->SetInput( connectedComponentFilter->GetOutput() );

  typedef itk::BinaryThresholdImageFilter< LabelImageType, LabelImageType >
    ConnectedComponentThresholdFilterType;
  ConnectedComponentThresholdFilterType::Pointer relabelThresholdFilter =
    ConnectedComponentThresholdFilterType::New();
  relabelThresholdFilter->SetInsideValue( INTERIOR );
  relabelThresholdFilter->SetOutsideValue( EXTERIOR );
  relabelThresholdFilter->SetLowerThreshold( 1 );
  relabelThresholdFilter->SetUpperThreshold( 1 );
  relabelThresholdFilter->SetInput( relabelComponentFilter->GetOutput() );
  relabelThresholdFilter->UpdateLargestPossibleRegion();

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
  typename SourcePadFilterType::Pointer sourcePadFilter = SourcePadFilterType::New();
  sourcePadFilter->SetPadLowerBound( lowerBound );
  sourcePadFilter->SetPadUpperBound( upperBound );
  sourcePadFilter->SetConstant( -1024 );
  sourcePadFilter->SetInput( inputReader->GetOutput() );
  try
    {
    sourcePadFilter->Update();
    }
  catch ( itk::ExceptionObject & except )
    {
    std::cerr << "Could not update sourcePadFilter\n";
    std::cerr << except << "\n";
    return EXIT_FAILURE;
    }

  typedef itk::ConstantPadImageFilter< LabelImageType, LabelImageType >
    BinaryPadFilterType;
  typename BinaryPadFilterType::Pointer binaryPadFilter = BinaryPadFilterType::New();
  binaryPadFilter->SetPadLowerBound( lowerBound );
  binaryPadFilter->SetPadUpperBound( upperBound );
  binaryPadFilter->SetConstant( EXTERIOR );
  binaryPadFilter->SetInput( relabelThresholdFilter->GetOutput() );
  try
    {
    binaryPadFilter->Update();
    }
  catch ( itk::ExceptionObject & except )
    {
    std::cerr << "Could not update binaryPadFilter\n";
    std::cerr << except << "\n";
    return EXIT_FAILURE;
    }

  LabelImageType* binaryImage = binaryPadFilter->GetOutput();

  AddNoseSphere( sphereCenter, sphereRadius, binaryImage,
                 sourcePadFilter->GetOutput(), segmentationThreshold,
                 INTERIOR, EXTERIOR, INFLOW_LOCAL );

  // Now fill in the volume below the lower cutoff seed
  LabelImageType::PointType cutoffITKPoint;
  cutoffITKPoint[0] = outflowCutoff[0];
  cutoffITKPoint[1] = outflowCutoff[1];
  cutoffITKPoint[2] = outflowCutoff[2];
  LabelImageType::IndexType sliceIndex;
  binaryImage->TransformPhysicalPointToIndex( cutoffITKPoint, sliceIndex );

  LabelImageType::RegionType belowRegion( binaryImage->GetLargestPossibleRegion() );
  LabelImageType::IndexType belowIndex( belowRegion.GetIndex() );
  LabelImageType::SizeType belowSize( belowRegion.GetSize() );
  belowSize[2] = (sliceIndex[2] - belowIndex[2]);
  belowRegion.SetSize( belowSize );
  itk::ImageRegionIterator< LabelImageType > belowIterator( binaryImage,
                                                            belowRegion );
  while ( !belowIterator.IsAtEnd() ) {
    belowIterator.Set( EXTERIOR );
    ++belowIterator;
  }

  // Now find the smallest part of the image that contains all the
  // geometry
  typedef itk::AutoCropImageFilter< LabelImageType, LabelImageType > CropFilterType;
  CropFilterType::Pointer cropper = CropFilterType::New();
  cropper->SetBackgroundValue( EXTERIOR );
  CropFilterType::InputImageSizeType pad = {{ 1, 1, 1 }};
  cropper->SetPadRadius( pad );
  cropper->SetInput( binaryImage );
  cropper->Update();

  binaryImage = cropper->GetOutput();

  // Get second Z slice and fill it with outflow boundary code
  int zSlice = binaryImage->GetLargestPossibleRegion().GetIndex()[2] + 1;

  // Add the outflow boundary to the image
  LabelImageType::RegionType region = binaryImage->GetLargestPossibleRegion();
  LabelImageType::IndexType index = region.GetIndex();
  LabelImageType::SizeType size = region.GetSize();
  for ( int j = index[1]; j < index[1] + (int)size[1]; ++j ) {
    for ( int i = index[0]; i < index[0] + (int)size[0]; ++i ) {
      LabelImageType::IndexType index = {{ i, j, zSlice }};
      if ( binaryImage->GetPixel( index ) == 0 ) {
        binaryImage->SetPixel( index, OUTFLOW_LOCAL );
      }

      // If the z-plane is set to 0 or the segmentation is not cut
      // off at a z-plane above 0, the cropped image will go all the
      // way to the bottom. In this case, we need to set the voxels
      // in the z-plane to exterior pixels
      index[2]--;
      binaryImage->SetPixel( index, EXTERIOR );
    }
  }

  // Pad by at least 1 voxel on all sides, but ensure that size
  // of each dimension is a multiple of 16
  typedef itk::ConstantPadImageFilter< LabelImageType, LabelImageType > PadFilterType;
  PadFilterType::Pointer padFilter = PadFilterType::New();
  padFilter->SetConstant( -1 );

  PadFilterType::SizeType padAmount;
  for ( int i = 0; i < 3; ++i ) {
    if ( size[i] % 16 != 0 ) {
      padAmount[i] = ( 16 - (size[i] % 16) );
    } else {
      padAmount[i] = 0;
    }
  }
  padFilter->SetPadUpperBound( padAmount );
  padFilter->SetInput( binaryImage );
  padFilter->Update();

  size = padFilter->GetOutput()->GetLargestPossibleRegion().GetSize();

  LabelImageType * paddedImage = padFilter->GetOutput();

  // Write the output
  typedef itk::ImageFileWriter< LabelImageType > OutputWriter;
  typename OutputWriter::Pointer writer = OutputWriter::New();
  writer->SetFileName( lbmImage.c_str() );
  writer->SetInput( paddedImage );

  // Set up explicit IO if VTK file is requested
  std::string extension = lbmImage.substr(lbmImage.size()-4, 4);
  if ( extension == ".vtk" )
    {
    typedef itk::VTKImageIO VTKIOType;
    typename VTKIOType::Pointer io = VTKIOType::New();
    io->SetFileTypeToASCII();
    writer->SetImageIO( io );
    }

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

    switch ( componentType )
      {
      case itk::ImageIOBase::UCHAR:
        std::cerr << "unsigned char pixel type not supported\n";
        result = EXIT_FAILURE;
        break;
      case itk::ImageIOBase::CHAR:
        std::cerr << "char pixel type not supported\n";
        result = EXIT_FAILURE;
        break;
      case itk::ImageIOBase::USHORT:
        std::cerr << "unsigned short pixel type not supported\n";
        result = EXIT_FAILURE;
        break;
      case itk::ImageIOBase::SHORT:
        result = DoIt( argc, argv, static_cast<short>(0) );
        break;
      case itk::ImageIOBase::UINT:
        std::cerr << "unsigned int pixel type not supported\n";
        result = EXIT_FAILURE;
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
