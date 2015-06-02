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

#include <vtkClipPolyData.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkSphere.h>
#include <vtkSmartPointer.h>

#include "../ITK/itkRasterizeSphereImageFilter.h"
#include "RemoveSphereCLP.h"

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
  typedef itk::RasterizeSphereImageFilter< ImageType >  SphereFilterType;

  // Creation of Reader and Writer filters
  typename ReaderType::Pointer reader = ReaderType::New();
  typename WriterType::Pointer writer  = WriterType::New();

  // Make the filter
  typename SphereFilterType::Pointer sphere = SphereFilterType::New();

  PointType SphereCent;

  SphereCent[0] = Center[0];
  SphereCent[1] = Center[1];
  SphereCent[2] = Center[2];
\
  sphere->SetSphereRadius( Radius );
  sphere->SetSphereCenter( SphereCent );

  reader->SetFileName( inputImage.c_str() );
  writer->SetFileName( outputImage.c_str() );
  writer->SetUseCompression(1);

  // Setup the boundary method
  sphere->SetInput(  reader->GetOutput() );
  sphere->Update();
  // Write the output
  writer->SetInput( sphere->GetOutput() );

  try
    {
    writer->Update();
    writer->Write();
    }
  catch (itk::ExceptionObject & except)
    {
    std::cerr << "Could not write file '" << outputImage << "\n";
    std::cerr << except << std::endl;
    return EXIT_FAILURE;
    }

  // Now trim the surface geometry
  vtkSmartPointer<vtkXMLPolyDataReader> surfaceReader =
    vtkSmartPointer<vtkXMLPolyDataReader>::New();
  surfaceReader->SetFileName( inputGeometry.c_str() );

  // Cut the surface
  vtkSmartPointer<vtkSphere> sphereFunction = vtkSmartPointer<vtkSphere>::New();
  // RAS to LPS transformation of sphere center
  sphereFunction->SetCenter( -Center[0], -Center[1], Center[2] );
  sphereFunction->SetRadius( Radius );

  vtkSmartPointer<vtkClipPolyData> clipper = vtkSmartPointer<vtkClipPolyData>::New();
  clipper->SetClipFunction( sphereFunction );
  clipper->SetInputConnection( surfaceReader->GetOutputPort() );

  vtkSmartPointer<vtkXMLPolyDataWriter> surfaceWriter =
    vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  surfaceWriter->SetFileName( outputGeometry.c_str() );
  surfaceWriter->SetInputConnection( clipper->GetOutputPort() );
  surfaceWriter->Update();

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
    GetImageType(inputImage, pixelType, componentType);
    // This filter handles all types
    switch( componentType )
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
