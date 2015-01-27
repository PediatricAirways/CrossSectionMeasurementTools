/*=============================================================================
//  --- Airway Segmenter ---+
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

// Local includes
#include "MeasureCrossSectionsCLP.h"
//#include "MeasureCrossSectionsConfig.h"

#include <itkImageIOBase.h>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageToVTKImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkResampleImageFilter.h>
#include <itkSpatialOrientationAdapter.h>

#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkThreshold.h>
#include <vtkXMLPolyDataWriter.h>

#include "vtkContourAtPointsFilter.h"


namespace
{
  /*******************************************************************
   ** Print all the settings.
   *******************************************************************/
  int OutputAllSettings(int argc, char* argv[])
  {
    PARSE_ARGS;

    std::cout << "-------------------------------------------------------------------------------"
              << std::endl;
    std::cout << "Parameter settings:" << std::endl;
    std::cout << "-------------------------------------------------------------------------------"
              << std::endl;
    std::cout << "heat flow image                  = " << heatFlowImage << std::endl;
    std::cout << "segmented surface geometry       = " << segmentedSurface << std::endl;
    std::cout << "output cross section geometry    = " << outputCrossSections << std::endl;

    std::cout << "-------------------------------------------------------------------------------"
              << std::endl;
    for (size_t i = 0; i < crossSectionPoints.size(); ++i)
      {
      std::cout << "crossSectionPoint " << i << " = "
                << crossSectionPoints[i][0] << ", "
                << crossSectionPoints[i][1] << ", "
                << crossSectionPoints[i][2] << std::endl;
      }
    if (crossSectionPoints.size())
      {
      std::cout << "-------------------------------------------------------------------------------"
                << std::endl;
      }

    return EXIT_SUCCESS;
  }

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

} // end anonymous namespace


/*******************************************************************/
/* Main function                                                   */
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

  // Automatic Resampling to RAI
  typename HeatFlowImageType::DirectionType originalImageDirection =
    originalImage->GetDirection();

  itk::SpatialOrientationAdapter adapter;
  typename HeatFlowImageType::DirectionType RAIDirection =
    adapter.ToDirectionCosines(itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI);

  bool shouldConvert = false;
  for ( int i = 0; i < 3; ++i )
    {
    for ( int j = 0; j < 3; ++j )
      {
      if (abs(originalImageDirection[i][j] - RAIDirection[i][j]) > 1e-6)
        {
        shouldConvert = true;
        break;
        }
      }
    }

  typedef itk::ResampleImageFilter< HeatFlowImageType, HeatFlowImageType > ResampleImageFilterType;
  typename ResampleImageFilterType::Pointer resampleFilter = ResampleImageFilterType::New();

  if ( shouldConvert )
    {
    typedef itk::IdentityTransform< double, DIMENSION > IdentityTransformType;

    // Figure out bounding box of rotated image
    double boundingBox[6] = { DBL_MAX, -DBL_MAX, DBL_MAX, -DBL_MAX, DBL_MAX, -DBL_MAX };
    typedef typename HeatFlowImageType::IndexType  IndexType;
    typedef typename HeatFlowImageType::RegionType RegionType;
    typedef typename HeatFlowImageType::PointType  PointType;

    RegionType region = originalImage->GetLargestPossibleRegion();
    IndexType lowerUpper[2];
    lowerUpper[0] = region.GetIndex();
    lowerUpper[1] = region.GetUpperIndex();

    for ( unsigned int i = 0; i < 8; ++i )
      {
      IndexType cornerIndex;
      cornerIndex[0] = lowerUpper[ (i & 1u) >> 0 ][0];
      cornerIndex[1] = lowerUpper[ (i & 2u) >> 1 ][1];
      cornerIndex[2] = lowerUpper[ (i & 4u) >> 2 ][2];
      std::cout << "cornerIndex: " << cornerIndex << std::endl;

      PointType point;
      originalImage->TransformIndexToPhysicalPoint( cornerIndex, point );
      boundingBox[0] = std::min( point[0], boundingBox[0] );
      boundingBox[1] = std::max( point[0], boundingBox[1] );
      boundingBox[2] = std::min( point[1], boundingBox[2] );
      boundingBox[3] = std::max( point[1], boundingBox[3] );
      boundingBox[4] = std::min( point[2], boundingBox[4] );
      boundingBox[5] = std::max( point[2], boundingBox[5] );
      }

    // Now transform the bounding box from physical space to index space
    PointType lowerPoint;
    lowerPoint[0] = boundingBox[0];
    lowerPoint[1] = boundingBox[2];
    lowerPoint[2] = boundingBox[4];

    PointType upperPoint;
    upperPoint[0] = boundingBox[1];
    upperPoint[1] = boundingBox[3];
    upperPoint[2] = boundingBox[5];

    typename HeatFlowImageType::Pointer dummyImage = HeatFlowImageType::New();
    dummyImage->SetOrigin( lowerPoint );
    dummyImage->SetSpacing( originalImage->GetSpacing() );
    dummyImage->SetLargestPossibleRegion( RegionType() );

    IndexType newLower, newUpper;
    dummyImage->TransformPhysicalPointToIndex( lowerPoint, newLower );
    dummyImage->TransformPhysicalPointToIndex( upperPoint, newUpper );

    RegionType outputRegion;
    outputRegion.SetIndex( newLower );
    outputRegion.SetUpperIndex( newUpper );

    // Find the minimum pixel value in the image. This will be used as
    // the default value in the resample filter.
    typedef itk::MinimumMaximumImageCalculator< HeatFlowImageType > MinMaxType;
    typename MinMaxType::Pointer minMaxCalculator = MinMaxType::New();
    minMaxCalculator->SetImage( originalImage );
    minMaxCalculator->Compute();

    resampleFilter->SetTransform( IdentityTransformType::New() );
    resampleFilter->SetInput( originalImage );
    resampleFilter->SetSize( outputRegion.GetSize() );
    resampleFilter->SetOutputOrigin( lowerPoint );
    resampleFilter->SetOutputSpacing( originalImage->GetSpacing() );
    resampleFilter->SetDefaultPixelValue( minMaxCalculator->GetMinimum() );
    resampleFilter->Update();

    originalImage = resampleFilter->GetOutput();
    }

  // Convert ITK image to VTK image
  typedef itk::ImageToVTKImageFilter< HeatFlowImageType > ITK2VTKFilterType;
  typename ITK2VTKFilterType::Pointer itk2vtkFilter = ITK2VTKFilterType::New();
  itk2vtkFilter->SetInput( originalImage );
  itk2vtkFilter->Update();

  // Threshold the image file to get rid of the NaNs
  vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
  threshold->ThresholdBetween(0.0, 1.0);
  threshold->AllScalarsOn();
  threshold->SetInputData( itk2vtkFilter->GetOutput() );

  // Create point set from markups
  vtkSmartPointer<vtkPoints> samplePoints = vtkSmartPointer<vtkPoints>::New();
  samplePoints->SetNumberOfPoints( crossSectionPoints.size() );
  for ( size_t i = 0; i < crossSectionPoints.size(); ++i )
    {
    samplePoints->SetPoint( i,
                            crossSectionPoints[i][0],
                            crossSectionPoints[i][1],
                            crossSectionPoints[i][2] );
    std::cout << "Setting point " << i << ": " << crossSectionPoints[i][0]
              << ", " << crossSectionPoints[i][1] << ", "
              << crossSectionPoints[i][2] << std::endl;
    }

  vtkSmartPointer<vtkPolyData> pointSet = vtkSmartPointer<vtkPolyData>::New();
  pointSet->SetPoints( samplePoints );

  // Compute contours
  vtkSmartPointer<vtkContourAtPointsFilter> contourFilter =
    vtkSmartPointer<vtkContourAtPointsFilter>::New();
  contourFilter->SetInputConnection( 0, threshold->GetOutputPort() );
  contourFilter->SetInputDataObject( 1, pointSet );

  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  transform->Scale( -1.0, -1.0, 1.0 );

  vtkSmartPointer<vtkTransformFilter> transformFilter =
    vtkSmartPointer<vtkTransformFilter>::New();
  transformFilter->SetTransform( transform );
  transformFilter->SetInputConnection( contourFilter->GetOutputPort() );

  // Write contours
  vtkSmartPointer<vtkXMLPolyDataWriter> pdWriter =
    vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  pdWriter->SetFileName( outputCrossSections.c_str() );
  pdWriter->SetInputConnection( transformFilter->GetOutputPort() );
  pdWriter->Write();

  return returnValue;
}


/*******************************************************************/
int main( int argc, char* argv[] )
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
