#include "itkImage.h"
#include "itkAirwayLaplaceSolutionFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include <cstdio>

 
 
int main(int argc, char *argv[])
{
  if( argc < 2 )
    { 
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << " inputImageFile NosePoint NoseVectorHead TrachPoint TrachVectorHead <outputFileName>. The points should be added as \'R A S\'" << std::endl;
    return EXIT_FAILURE;
    }

  std::string outputFileName = "out.nrrd";
  if ( argc > 14 )
   {
     outputFileName = argv[14];
   }
 
  typedef itk::Image< unsigned short, 3 >      ImageType;
  typedef itk::Image< float, 3 >               TOutputImage;
  typedef itk::ImageFileReader< ImageType >    ReaderType;
  typedef itk::ImageFileWriter< TOutputImage > WriterType;
  typedef ImageType::PointType                 PointType;

  // Input Parsing
  double    x, y, z;
  PointType nP, nV;
  PointType tP, tV;

  // printf( "Enter Nasal Point \'X Y Z\': " );
  // scanf( "%lf %lf %lf", &x, &y, &z );
  nP[0] = -atof( argv[2] );
  nP[1] = -atof( argv[3] );
  nP[2] =  atof( argv[4] );
  // printf("%lf %lf %lf\n", nP[0], nP[1], nP[2]);

  // printf( "Enter Nasal Vector \'X Y Z\': " );
  // scanf( "%lf %lf %lf", &x, &y, &z );
  nV[0] = -atof( argv[5] );
  nV[1] = -atof( argv[6] );
  nV[2] =  atof( argv[7] );

  // printf( "Enter Trachea Point \'X Y Z\': " );
  // scanf( "%lf %lf %lf", &x, &y, &z );
  tP[0] = -atof( argv[8] );
  tP[1] = -atof( argv[9] );
  tP[2] =  atof( argv[10]);

  // printf( "Enter Trachea Vector \'X Y Z\': " );
  // scanf( "%lf %lf %lf", &x, &y, &z );
  tV[0] = -atof( argv[11] );
  tV[1] = -atof( argv[12] );
  tV[2] =  atof( argv[13] );


  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( argv[1] );
 
  typedef itk::AirwayLaplaceSolutionFilter < ImageType, TOutputImage >
    AirwayLaplaceSolutionFilterType;
 
  AirwayLaplaceSolutionFilterType::Pointer solutionFilter
    = AirwayLaplaceSolutionFilterType::New();

  solutionFilter->SetInput( reader->GetOutput() );
  solutionFilter->SetNosePoint(   nP );
  solutionFilter->SetNoseVector(  nV );
  solutionFilter->SetTrachPoint(  tP );
  solutionFilter->SetTrachVector( tV );
  solutionFilter->Update();
 


 WriterType::Pointer writer = WriterType::New();
  writer->SetFileName ( outputFileName.c_str() );
  writer->SetInput( solutionFilter->GetOutput() );
  writer->Write();
 
  return EXIT_SUCCESS;
}