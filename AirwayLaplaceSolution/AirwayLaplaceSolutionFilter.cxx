/*=========================================================================

  Program:   Slicer
  Language:  C++
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "itkImageFileWriter.h"
#include "itkImageFileReader.h"
#include "../../Slicer/Base/CLI/itkPluginUtilities.h"

#include "itkAirwayLaplaceSolutionFilter.h"
#include "AirwayLaplaceSolutionFilterCLP.h"

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

template <class T>
int DoIt( int argc, char * argv[], T )
{
  PARSE_ARGS;

  const unsigned int Dimension = 3;

  typedef T InputPixelType;
  typedef itk::Image< float, 3 >                 OutputImageType;
  typedef itk::Image<InputPixelType,  Dimension> InputImageType;

  typedef typename InputImageType::PointType PointType;

  PointType NPoint, NoseVector, TPoint, TracheaVector;
  double    NoseLength, TracheaLength;


  NPoint[0] = NasalPoint[0];
  NPoint[1] = NasalPoint[1];
  NPoint[2] = NasalPoint[2];

  TPoint[0] = TrachealPoint[0];
  TPoint[1] = TrachealPoint[1];
  TPoint[2] = TrachealPoint[2];
  // Calculate the Nose and Trachea plane Normals
  NoseVector[0]     = NasalVectorHead[0] - NPoint[0];
  NoseVector[1]     = NasalVectorHead[1] - NPoint[1];
  NoseVector[2]     = NasalVectorHead[2] - NPoint[2];

  TracheaVector[0]  = TrachealVectorHead[0] - TPoint[0];
  TracheaVector[1]  = TrachealVectorHead[1] - TPoint[1];
  TracheaVector[2]  = TrachealVectorHead[2] - TPoint[2];

  NoseLength        = sqrt( NoseVector[0]*NoseVector[0]       + NoseVector[1]*NoseVector[1]       + NoseVector[2]*NoseVector[2]       );
  TracheaLength     = sqrt( TracheaVector[0]*TracheaVector[0] + TracheaVector[1]*TracheaVector[1] + TracheaVector[2]*TracheaVector[2] );

  NoseVector[0]     = NoseVector[0]/NoseLength;
  NoseVector[1]     = NoseVector[1]/NoseLength;
  NoseVector[2]     = NoseVector[2]/NoseLength;

  TracheaVector[0]  = TracheaVector[0]/TracheaLength;
  TracheaVector[1]  = TracheaVector[1]/TracheaLength;
  TracheaVector[2]  = TracheaVector[2]/TracheaLength;


  // readers/writers
  typedef itk::ImageFileReader<InputImageType>  ReaderType;
  typedef itk::ImageFileWriter<OutputImageType> WriterType;

  // define the boundary filter
  typedef itk::AirwayLaplaceSolutionFilter< InputImageType, OutputImageType >  AirwayFilterType;

  // Creation of Reader and Writer filters
  typename ReaderType::Pointer reader = ReaderType::New();
  typename WriterType::Pointer writer  = WriterType::New();

  // Create the filter
  typename AirwayFilterType::Pointer  bound = AirwayFilterType::New();

  // Setup the input and output files

  bound->SetNosePoint(    NPoint        );
  bound->SetNoseVector(   NoseVector    );
  bound->SetTrachPoint(   TPoint        );
  bound->SetTrachVector(  TracheaVector );

  reader->SetFileName( inputImage.c_str() );
  writer->SetFileName( outputImage.c_str() );
  writer->SetUseCompression(1);

  // Setup the boundary method
  bound->SetInput(  reader->GetOutput() );

  // Write the output
  writer->SetInput( bound->GetOutput() );
  writer->Update();
  writer->Write();

  return EXIT_SUCCESS;

}

} // end of anonymous namespace

int main( int argc, char * argv[] )
{
  // DoIt(argc, argv);
  PARSE_ARGS;


  itk::ImageIOBase::IOPixelType     pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  try
    {
    itk::GetImageType(inputImage, pixelType, componentType);
    // This filter handles all types
    switch( pixelType )
      {
      case itk::ImageIOBase::UCHAR:
        return DoIt( argc, argv, static_cast<unsigned char>(0) );
        break;
      case itk::ImageIOBase::CHAR:
        return DoIt( argc, argv, static_cast<char>(0) );
        break;
      case itk::ImageIOBase::USHORT:
        return DoIt( argc, argv, static_cast<unsigned short>(0) );
        break;
      case itk::ImageIOBase::SHORT:
        return DoIt( argc, argv, static_cast<short>(0) );
        break;
      case itk::ImageIOBase::UINT:
        return DoIt( argc, argv, static_cast<unsigned int>(0) );
        break;
      case itk::ImageIOBase::INT:
        return DoIt( argc, argv, static_cast<int>(0) );
        break;
      case itk::ImageIOBase::ULONG:
        return DoIt( argc, argv, static_cast<unsigned long>(0) );
        break;
      case itk::ImageIOBase::LONG:
        return DoIt( argc, argv, static_cast<long>(0) );
        break;
      case itk::ImageIOBase::FLOAT:
        return DoIt( argc, argv, static_cast<float>(0) );
        break;
      case itk::ImageIOBase::DOUBLE:
        return DoIt( argc, argv, static_cast<double>(0) );
        break;
      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
      default:
        std::cout << "unknown component type" << std::endl;
        break;
      }
    }

  catch( itk::ExceptionObject & excep )
    {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
