/*=============================================================================
//  --- Threshold Laplace Solution ---+
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
//  Authors: Cory Quammen
=============================================================================*/

#include "ThresholdLaplaceSolutionCLP.h"

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageToVTKImageFilter.h>

#include <vtkSmartPointer.h>
#include <vtkThreshold.h>
#include <vtkXMLUnstructuredGridWriter.h>

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
  /**  */
  /*******************************************************************/

} // end anonymous namespace

/*******************************************************************/
/* Does all the work                                               */
/*******************************************************************/
template< typename T >
int DoIt( int argc, char* argv[], T )
{
  PARSE_ARGS;

  int returnValue = EXIT_SUCCESS;

  typedef T TPixelType;
  const unsigned char DIMENSION = 3;

  typedef itk::Image< TPixelType, DIMENSION > HeatFlowImageType;

  // Read heatflow image
  typedef itk::ImageFileReader< HeatFlowImageType > HeatFlowReaderType;
  typename HeatFlowReaderType::Pointer heatFlowReader = HeatFlowReaderType::New();
  heatFlowReader->SetFileName( heatFlowImage.c_str() );
  heatFlowReader->Update();

  typename HeatFlowImageType::Pointer originalImage =
    heatFlowReader->GetOutput();

  // First verify that image is in LPS orientation
  typename HeatFlowImageType::DirectionType originalImageDirection =
    originalImage->GetDirection();

  typename HeatFlowImageType::DirectionType LPSDirection;
  LPSDirection.SetIdentity();

  bool notLPS = false;
  for ( int i = 0; i < 3; ++i )
    {
    for ( int j = 0; j < 3; ++j )
      {
      if (abs(originalImageDirection[i][j] - LPSDirection[i][j]) > 1e-6)
        {
        notLPS = true;
        break;
        }
      }
    }

  if ( notLPS )
    {
    std::cerr << "Heat flow image is not in LPS coordinate system.\n";
    return EXIT_FAILURE;
    }

  // Convert ITK image to VTK image
  typedef itk::ImageToVTKImageFilter< HeatFlowImageType > ITK2VTKFilterType;
  typename ITK2VTKFilterType::Pointer itk2vtkFilter = ITK2VTKFilterType::New();
  itk2vtkFilter->SetInput( originalImage );
  itk2vtkFilter->Update();

  vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
  threshold->ThresholdBetween(0.0, 1.0);
  threshold->AllScalarsOff();
  threshold->SetInputData( itk2vtkFilter->GetOutput() );

  vtkSmartPointer<vtkXMLUnstructuredGridWriter> writer =
    vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
  writer->SetFileName(thresholdOutput.c_str());
  writer->SetInputConnection( threshold->GetOutputPort() );
  writer->Write();

  return returnValue;
}

/*******************************************************************/
// Threshold the Laplace solution image between 0 and 1. Produces an
// unstructured grid.
int main( int argc, char* argv[])
{
  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType     inputPixelType;
  itk::ImageIOBase::IOComponentType inputComponentType;

  int returnValue = EXIT_SUCCESS;

  try
    {
    GetImageType( heatFlowImage, inputPixelType, inputComponentType );

    switch ( inputComponentType )
      {

      case itk::ImageIOBase::FLOAT:
        returnValue = DoIt( argc, argv, static_cast< float >( 0 ) );
        break;

      case itk::ImageIOBase::DOUBLE:
        returnValue = DoIt( argc, argv, static_cast< double >( 0 ) );
        break;

      default:
        std::cerr << "Unknown component type" << std::endl;
        break;
      }

    }
  catch ( itk::ExceptionObject & except )
    {
    std::cerr << argv[0] << ": exception caught!" << std::endl;
    std::cerr << except << std::endl;

    returnValue = EXIT_FAILURE;
    }

  return returnValue;  
}
