#include "itkImage.h"
#include "itkExcludeSphereImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include <cstdio>

 
 
int main(int argc, char *argv[])
{
  if( argc < 2 )
    { 
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << " inputImageFile SphereCenterPoint SphereRadius <outputFileName>. The point should be added as \'R A S\'" << std::endl;
    return EXIT_FAILURE;
    }

  std::string outputFileName = "/Users/jonathankylstra/DATA/1092/1092_CUT.nrrd";
  // if ( argc > 6 )
  //  {
  //    outputFileName = argv[6];
  //  }
 
  typedef itk::Image< unsigned short, 3 >      ImageType;
  typedef itk::ImageFileReader< ImageType >    ReaderType;
  typedef itk::ImageFileWriter< ImageType >    WriterType;
  typedef ImageType::PointType                 PointType;

  // Input Parsing
  double    radius;
  PointType p;
  // /Users/jonathankylstra/DATA/1092/1092_OUTPUT.nrrd -6.09449157051641 213.85593591755 354.722187630732 32



  p[0] = 6.09449157051641;
  p[1] = -213.85593591755;
  p[2] = 354.722187630732;

  radius = 32;

  std::cout << radius << ' ' << p[0] << ' ' << p[1] << ' ' << p[2] << std::endl;

  std::string inputFileName = "/Users/jonathankylstra/DATA/1092/1092_OUTPUT.nrrd";

  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( inputFileName );
 
  typedef itk::ExcludeSphereImageFilter < ImageType >
    ExcludeSphereImageFilterType;
 
  ExcludeSphereImageFilterType::Pointer excludeFilter
    = ExcludeSphereImageFilterType::New();

  excludeFilter->SetInput( reader->GetOutput() );
  excludeFilter->SetSphereRadius( radius );
  excludeFilter->SetSphereCenter( p );
  excludeFilter->Update();
 


  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName ( outputFileName.c_str() );
  writer->SetInput( excludeFilter->GetOutput() );
  writer->Write();
 
  return EXIT_SUCCESS;
}