#include "itkImage.h"
#include "itkAirwayLaplaceBoundaryImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include <stdio.h>
 
 
int main(int argc, char *argv[])
{
  if( argc < 2 )
    { 
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << " inputImageFile <outputFileName>" << std::endl;
    return EXIT_FAILURE;
    }

  std::string outputFileName = "out.nrrd";
  if ( argc > 2 )
   {
     outputFileName = argv[2];
   }
 
  typedef itk::Image< unsigned short, 3 >    ImageType;
  typedef itk::ImageFileReader< ImageType > ReaderType;
  typedef itk::ImageFileWriter< ImageType > WriterType;
  typedef ImageType::PointType              PointType;


  // Input Parsing
  PointType nP, nV;
  PointType tP, tV;

  // printf( "Enter Nasal Point \'X Y Z\': " );
  // scanf( "%lf %lf %lf", &x, &y, &z );
  nP[0] =    0.0;
  nP[1] = -236.0;
  nP[2] = -100.0;

  // printf( "Enter Nasal Vector \'X Y Z\': " );
  // scanf( "%lf %lf %lf", &x, &y, &z );
  nV[0] =  0.0;
  nV[1] =  1.0;
  nV[2] =  0.0;

  // printf( "Enter Trachea Point \'X Y Z\': " );
  // scanf( "%lf %lf %lf", &x, &y, &z );
  tP[0] =   -5.011;
  tP[1] = -124.864;
  tP[2] = -281.11;

  // printf( "Enter Trachea Vector \'X Y Z\': " );
  // scanf( "%lf %lf %lf", &x, &y, &z );
  tV[0] = 0.0;
  tV[1] = 0.0;
  tV[2] = 1.0;


  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( argv[1] );
 
  typedef itk::AirwayLaplaceBoundaryImageFilter < ImageType >
    AirwayLaplaceBoundaryImageFilterType;
 
  AirwayLaplaceBoundaryImageFilterType::Pointer boundaryFilter
    = AirwayLaplaceBoundaryImageFilterType::New();

  boundaryFilter->SetInput( reader->GetOutput() );
  boundaryFilter->SetNasalPoint( nP );
  boundaryFilter->SetNasalVector( nV );
  boundaryFilter->SetTracheaPoint( tP );
  boundaryFilter->SetTracheaVector( tV );
  boundaryFilter->Update();
 


 WriterType::Pointer writer = WriterType::New();
  writer->SetFileName ( outputFileName.c_str() );
  writer->SetInput( boundaryFilter->GetOutput() );
  writer->Write();
 
  return EXIT_SUCCESS;
}
