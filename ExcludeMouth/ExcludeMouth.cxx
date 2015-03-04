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

#include "itkExcludeSphereImageFilter.h"
#include "ExcludeMouthCLP.h"

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
  typedef itk::Image< InputPixelType, Dimension > ImageType;
  typedef itk::ImageFileReader< ImageType >       ReaderType;
  typedef itk::ImageFileWriter< ImageType >       WriterType;
  typedef typename ImageType::PointType           PointType;

  // define the filter
  typedef itk::ExcludeSphereImageFilter< ImageType >  ExcludeFilterType;

  // Creation of Reader and Writer filters
  typename ReaderType::Pointer reader = ReaderType::New();
  typename WriterType::Pointer writer  = WriterType::New();

  // Make the filter
  typename ExcludeFilterType::Pointer exclude = ExcludeFilterType::New();

  PointType SphereCent;

  SphereCent[0] = Center[0];
  SphereCent[1] = Center[1];
  SphereCent[2] = Center[2];


  exclude->SetSphereRadius( Radius );
  exclude->SetSphereCenter( SphereCent );

  reader->SetFileName( inputImage.c_str() );
  writer->SetFileName( outputImage.c_str() );
  writer->SetUseCompression(1);

  // Setup the boundary method
  exclude->SetInput(  reader->GetOutput() );

  // Write the output
  writer->SetInput( exclude->GetOutput() );
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
