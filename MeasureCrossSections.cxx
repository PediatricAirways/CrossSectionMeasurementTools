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

#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkContourFilter.h>
#include <vtkCutter.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkGenericCell.h>
#include <vtkMassProperties.h>
#include <vtkOctreePointLocator.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkSmartPointer.h>
#include <vtkStripper.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkTriangle.h>
#include <vtkTriangleFilter.h>
#include <vtkThreshold.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLPolyDataReader.h>
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

  vtkSmartPointer<vtkXMLPolyDataReader> segmentationSurfaceReader =
    vtkSmartPointer<vtkXMLPolyDataReader>::New();
  segmentationSurfaceReader->SetFileName( segmentedSurface.c_str() );
  segmentationSurfaceReader->Update();

  // Slicer assumes poly data is in RAS space. We are operating in
  // LPS, so we need to convert the surface hear.
  vtkSmartPointer<vtkTransform> RASToLPSTransform = vtkSmartPointer<vtkTransform>::New();
  RASToLPSTransform->Scale( -1.0, -1.0, 1.0 );

  vtkSmartPointer<vtkTransformFilter> transformedSegmentationSurface =
    vtkSmartPointer<vtkTransformFilter>::New();
  transformedSegmentationSurface->SetTransform( RASToLPSTransform );
  transformedSegmentationSurface->
    SetInputConnection( segmentationSurfaceReader->GetOutputPort() );

  // Input images are in LPS coordinate system while segmented surface
  // is in RAS. Do everything in LPS.

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

  // Threshold the image file to get rid of the NaNs
  vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
  threshold->ThresholdBetween(0.0, 1.0);
  threshold->AllScalarsOff();
  threshold->SetInputData( itk2vtkFilter->GetOutput() );
  threshold->Update();

  vtkUnstructuredGrid* thresholded = threshold->GetOutput();

  // Now create a set of 1000 contours along the heat flow image
  vtkSmartPointer<vtkContourFilter> contourFilter =
    vtkSmartPointer<vtkContourFilter>::New();
  contourFilter->GenerateValues( 1000, 0.0, 1.0 );
  contourFilter->SetInputConnection( threshold->GetOutputPort() );
  contourFilter->Update();

  vtkPolyData* contours = contourFilter->GetOutput();

  // For each query point, find nearest cross section
  vtkSmartPointer<vtkOctreePointLocator> octree =
    vtkSmartPointer<vtkOctreePointLocator>::New();
  octree->SetDataSet( contours );
  octree->BuildLocator();

  size_t numPoints = crossSectionPoints.size();

  vtkSmartPointer<vtkDoubleArray> centerOfMassInfo = vtkSmartPointer<vtkDoubleArray>::New();
  centerOfMassInfo->SetName( "center of mass" );
  centerOfMassInfo->SetNumberOfComponents( 3 );
  centerOfMassInfo->SetNumberOfTuples( numPoints );

  vtkSmartPointer<vtkDoubleArray> averageNormalInfo = vtkSmartPointer<vtkDoubleArray>::New();
  averageNormalInfo->SetName( "normal" );
  averageNormalInfo->SetNumberOfComponents( 3 );
  averageNormalInfo->SetNumberOfTuples( numPoints );

  vtkSmartPointer<vtkDoubleArray> areaInfo = vtkSmartPointer<vtkDoubleArray>::New();
  areaInfo->SetName( "area" );
  areaInfo->SetNumberOfComponents( 1 );
  areaInfo->SetNumberOfTuples( numPoints );

  vtkSmartPointer<vtkDoubleArray> perimeterInfo = vtkSmartPointer<vtkDoubleArray>::New();
  perimeterInfo->SetName( "perimeter" );
  perimeterInfo->SetNumberOfComponents( 1 );
  perimeterInfo->SetNumberOfTuples( numPoints );

  size_t numQueryPoints = crossSectionPoints.size();
  vtkSmartPointer<vtkPoints> samplePoints = vtkSmartPointer<vtkPoints>::New();
  samplePoints->SetNumberOfPoints( crossSectionPoints.size() );

  vtkSmartPointer<vtkAppendPolyData> appender =
    vtkSmartPointer<vtkAppendPolyData>::New();

  for ( size_t inputPtId = 0; inputPtId < crossSectionPoints.size(); ++inputPtId )
    {
    double queryPt[3];
    queryPt[0] = crossSectionPoints[inputPtId][0];
    queryPt[1] = crossSectionPoints[inputPtId][1];
    queryPt[2] = crossSectionPoints[inputPtId][2];

    vtkIdType closestPtId = octree->FindClosestPoint( queryPt );

    // Get the scalar associated with the closest point
    std::cout << "queryPt: " << queryPt[0] << ", "
              << queryPt[1] << ", " << queryPt[2] << std::endl;

    double closestPt[3];
    contours->GetPoint( closestPtId, closestPt );
    std::cout << "closestPt: " << closestPt[0] << ", " <<
      closestPt[1] << ", " << closestPt[2] << std::endl;
    std::cout << "closestPt id: " << closestPtId << std::endl;
    if ( closestPtId >= 0 )
      {
      double nearestScalar = contours->GetPointData()->GetScalars()->GetTuple1( closestPtId );
      std::cout << "nearestScalar: " << nearestScalar << std::endl;

      // Extract contour for the nearest scalar value
      vtkSmartPointer<vtkThreshold> scalarThreshold = vtkSmartPointer<vtkThreshold>::New();
      scalarThreshold->ThresholdBetween( nearestScalar - 1e-5, nearestScalar + 1e-5 );
      scalarThreshold->AllScalarsOn();
      scalarThreshold->SetInputConnection( contourFilter->GetOutputPort() );
      //scalarThreshold->Update();

      vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter =
        vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
      surfaceFilter->SetInputConnection( scalarThreshold->GetOutputPort() );
      surfaceFilter->Update();

      // Center of mass of surface elements is the average of the
      // centers of the surface triangles weighted by the triangle area.
      vtkPolyData* pd = surfaceFilter->GetOutput();
      vtkCellArray* ca = pd->GetPolys();

      ca->InitTraversal();
      vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();
      vtkSmartPointer<vtkIdList> ptList = vtkSmartPointer<vtkIdList>::New();
      double centerOfMass[3] = { 0.0, 0.0, 0.0 };
      double averageNormal[3] = { 0.0, 0.0, 0.0 };
      double totalArea = 0.0;
      while ( ca->GetNextCell( ptList ) )
        {
        double p0[3], p1[3], p2[3];
        pd->GetPoint( ptList->GetId( 0 ), p0 );
        pd->GetPoint( ptList->GetId( 1 ), p1 );
        pd->GetPoint( ptList->GetId( 2 ), p2 );
        double area = vtkTriangle::TriangleArea( p0, p1, p2 );
        totalArea += area;

        double center[3], normal[3];
        vtkTriangle::TriangleCenter( p0, p1, p2, center );
        vtkTriangle::ComputeNormal( p0, p1, p2, normal );

        for ( int i = 0; i < 3; ++i )
          {
          centerOfMass[i]  += area * center[i];
          averageNormal[i] += area * normal[i];
          }
        }

      centerOfMass[0] /= totalArea;
      centerOfMass[1] /= totalArea;
      centerOfMass[2] /= totalArea;

      std::cout << "com: "
                << centerOfMass[0] << ", "
                << centerOfMass[1] << ", "
                << centerOfMass[2] << std::endl;

      centerOfMassInfo->SetTupleValue( inputPtId, centerOfMass );

      averageNormal[0] /= totalArea;
      averageNormal[1] /= totalArea;
      averageNormal[2] /= totalArea;

      std::cout << "normal: "
                << averageNormal[0] << ", "
                << averageNormal[1] << ", "
                << averageNormal[2] << std::endl;

      averageNormalInfo->SetTupleValue( inputPtId, averageNormal );

      // Now cut the polygonal model from the segmentation by the plane
      // defined by the center of mass and normal
      vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
      plane->SetOrigin( centerOfMass );
      plane->SetNormal( averageNormal );

      vtkSmartPointer<vtkCutter> cutter = vtkSmartPointer<vtkCutter>::New();
      cutter->SetCutFunction( plane );
      cutter->GenerateCutScalarsOn();
      cutter->SetNumberOfContours( 0 );
      cutter->SetValue( 0, 0.0 );
      cutter->SetInputConnection( transformedSegmentationSurface->GetOutputPort() );
      cutter->Update();

      vtkSmartPointer<vtkPolyDataConnectivityFilter> planarCutConnectivity =
        vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
      planarCutConnectivity->SetExtractionModeToClosestPointRegion();
      planarCutConnectivity->SetClosestPoint( centerOfMass );
      planarCutConnectivity->SetInputConnection( cutter->GetOutputPort() );

      // Convert planar cut into line strips
      vtkSmartPointer<vtkStripper> stripper = vtkSmartPointer<vtkStripper>::New();
      stripper->SetInputConnection( planarCutConnectivity->GetOutputPort() );
      stripper->Update();

      // Convert the poly line from the cut to a polygon
      vtkPolyData* cut = stripper->GetOutput();

      std::cout << "cut cells: " << cut->GetNumberOfCells() << std::endl;

      if ( cut->GetNumberOfCells() == 0 )
        {
        double zero = 0.0;
        areaInfo->SetTupleValue( inputPtId, &zero );
        perimeterInfo->SetTupleValue( inputPtId, &zero );
        continue;
        }

      vtkCell* polyLine = cut->GetCell( 0 ); // Assumes one cut object
      vtkSmartPointer<vtkPolyData> polygon = vtkSmartPointer<vtkPolyData>::New();
      polygon->Allocate();
      polygon->SetPoints( cut->GetPoints() );
      polygon->InsertNextCell( VTK_POLYGON, polyLine->GetPointIds() );

      vtkSmartPointer<vtkTriangleFilter> triangulate =
        vtkSmartPointer<vtkTriangleFilter>::New();
      triangulate->SetInputData( polygon );
      triangulate->Update();

      appender->AddInputConnection( triangulate->GetOutputPort() );

      // Now measure the surface area of the cross section
      vtkSmartPointer<vtkMassProperties> massProperties =
        vtkSmartPointer<vtkMassProperties>::New();
      massProperties->SetInputConnection( triangulate->GetOutputPort() );
      massProperties->Update();
      double surfaceArea = massProperties->GetSurfaceArea();

      std::cout << "surface area: " << surfaceArea << std::endl;

      areaInfo->SetTupleValue( inputPtId, &surfaceArea );

      // Now compute the perimeter of the cross section.
      double perimeter = 0.0;
      vtkIdList* pointIds = polyLine->GetPointIds();
      int numPolyLinePoints = pointIds->GetNumberOfIds();
      if ( numPolyLinePoints > 1 )
        {
        for (vtkIdType i = 0; i < numPolyLinePoints; ++i)
          {
          vtkIdType ptId0 = pointIds->GetId( i );
          vtkIdType ptId1 = pointIds->GetId( (i + 1) % numPolyLinePoints );
          double pt0[3], pt1[3];
          polygon->GetPoints()->GetPoint( ptId0, pt0 );
          polygon->GetPoints()->GetPoint( ptId1, pt1 );

          perimeter += sqrt( vtkMath::Distance2BetweenPoints( pt0, pt1 ) );
          }
        }

      perimeterInfo->SetTupleValue( inputPtId, &perimeter );

      }
    }

  // VTK data has no associated transform, so Slicer assumes it is
  // in the RAS coordinate space. We are operating in LPS space, so we
  // need to convert to RAS here.
  vtkSmartPointer<vtkTransform> LPSToRASTransform =
    vtkSmartPointer<vtkTransform>::New();
  LPSToRASTransform->Scale( -1.0, -1.0, 1.0 );

  vtkSmartPointer<vtkTransformFilter> transformFilter =
    vtkSmartPointer<vtkTransformFilter>::New();
  transformFilter->SetTransform( LPSToRASTransform );
  transformFilter->SetInputConnection( appender->GetOutputPort() );
  transformFilter->Update();

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
