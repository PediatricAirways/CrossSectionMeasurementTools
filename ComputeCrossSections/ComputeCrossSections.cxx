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
#include "ComputeCrossSectionsCLP.h"

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageToVTKImageFilter.h>

#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCleanPolyData.h>
#include <vtkConnectivityFilter.h>
#include <vtkContourCompleter.h>
#include <vtkContourFilter.h>
#include <vtkContourTriangulator.h>
#include <vtkCutter.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkDelimitedTextWriter.h>
#include <vtkDoubleArray.h>
#include <vtkFeatureEdges.h>
#include <vtkFieldData.h>
#include <vtkLine.h>
#include <vtkMassProperties.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkPolyLine.h>
#include <vtkSmartPointer.h>
#include <vtkStripper.h>
#include <vtkTable.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkTriangle.h>
#include <vtkTriangleFilter.h>
#include <vtkThreshold.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>


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
    std::cout << "output comma-delimited text file = " << outputCSVFile << std::endl;

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

  /*******************************************************************/
  /**  */
  /*******************************************************************/

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

  vtkSmartPointer<vtkAlgorithm> reader;
  std::string vtkExtension( ".vtk" );
  std::string vtpExtension( ".vtp" );
  if ( std::equal( vtkExtension.rbegin(), vtkExtension.rend(),
                   segmentedSurface.rbegin() ) )
    {
    std::cout << "Reading VTK file\n";
    vtkSmartPointer<vtkPolyDataReader> surfaceReader =
      vtkSmartPointer<vtkPolyDataReader>::New();
    surfaceReader->SetFileName( segmentedSurface.c_str() );
    surfaceReader->Update();
    reader = surfaceReader;
    }
  else if ( std::equal( vtpExtension.rbegin(), vtpExtension.rend(),
                        segmentedSurface.rbegin() ) )
    {
    std::cout << "Reading VTP file\n";
    vtkSmartPointer<vtkXMLPolyDataReader> surfaceReader =
      vtkSmartPointer<vtkXMLPolyDataReader>::New();
    surfaceReader->SetFileName( segmentedSurface.c_str() );
    surfaceReader->Update();
    reader = surfaceReader;
    }
  else
    {
    std::cerr << "Unknown file extension in file '" << segmentedSurface << "'. Exiting.\n";
    return EXIT_FAILURE;
    }

  // Slicer assumes poly data is in RAS space. We are operating in
  // LPS, so we need to convert the surface here.
  vtkSmartPointer<vtkTransform> RASToLPSTransform = vtkSmartPointer<vtkTransform>::New();
  RASToLPSTransform->Scale( -1.0, -1.0, 1.0 );

  vtkSmartPointer<vtkTransformFilter> transformedSegmentationSurface =
    vtkSmartPointer<vtkTransformFilter>::New();
  transformedSegmentationSurface->SetTransform( RASToLPSTransform );
  transformedSegmentationSurface->
    SetInputConnection( reader->GetOutputPort() );

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
  std::cout << "Thresholding... ";
  vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
  threshold->ThresholdBetween(0.0, 1.0);
  threshold->AllScalarsOff();
  threshold->SetInputData( itk2vtkFilter->GetOutput() );
  threshold->Update();
  std::cout << "done\n";

  // Now create a set of 1000 contours along the heat flow image
  std::cout << "Creating contours... ";
  vtkIdType numContours = 1000;
  vtkSmartPointer<vtkContourFilter> contourFilter =
    vtkSmartPointer<vtkContourFilter>::New();
  contourFilter->GenerateValues( numContours, 0.0, 1.0 );
  contourFilter->SetInputConnection( threshold->GetOutputPort() );
  contourFilter->Update();
  std::cout << "done\n";

  // Point data with heat flow values
  vtkSmartPointer<vtkDoubleArray> heatValues = vtkSmartPointer<vtkDoubleArray>::New();
  heatValues->SetName( "heat" );
  heatValues->SetNumberOfComponents( 1 );

  // Cell data with contour ID
  vtkSmartPointer<vtkIdTypeArray> contourIDs = vtkSmartPointer<vtkIdTypeArray>::New();
  contourIDs->SetName( "contour ID" );
  contourIDs->SetNumberOfComponents( 1 );

  // Field data containing meta data about the cross sections. One
  // entry for each cross-section is stored for each of the arrays
  // centerOfMassInfo, averageNormalInfo, areaInfo, and perimeterInfo.
  vtkSmartPointer<vtkDoubleArray> centerOfMassInfo = vtkSmartPointer<vtkDoubleArray>::New();
  centerOfMassInfo->SetName( "center of mass" );
  centerOfMassInfo->SetNumberOfComponents( 3 );
  centerOfMassInfo->SetNumberOfTuples( numContours );

  vtkSmartPointer<vtkDoubleArray> averageNormalInfo = vtkSmartPointer<vtkDoubleArray>::New();
  averageNormalInfo->SetName( "normal" );
  averageNormalInfo->SetNumberOfComponents( 3 );
  averageNormalInfo->SetNumberOfTuples( numContours );

  vtkSmartPointer<vtkDoubleArray> areaInfo = vtkSmartPointer<vtkDoubleArray>::New();
  areaInfo->SetName( "area" );
  areaInfo->SetNumberOfComponents( 1 );
  areaInfo->SetNumberOfTuples( numContours );

  vtkSmartPointer<vtkDoubleArray> perimeterInfo = vtkSmartPointer<vtkDoubleArray>::New();
  perimeterInfo->SetName( "perimeter" );
  perimeterInfo->SetNumberOfComponents( 1 );
  perimeterInfo->SetNumberOfTuples( numContours );

  vtkSmartPointer<vtkAppendPolyData> appender =
    vtkSmartPointer<vtkAppendPolyData>::New();

  bool firstCrossSection = true;
  double previousCenterlinePoint[3] = {0.0, 0.0, 0.0};

  for ( vtkIdType contourID = numContours-1; contourID >= 0; --contourID )
    {
    double scalar = contourFilter->GetValue( contourID );

    std::cout << "Processing contour " << contourID << " - " << scalar <<std::endl;

    // Extract contour for the nearest scalar value
    vtkSmartPointer<vtkThreshold> scalarThreshold = vtkSmartPointer<vtkThreshold>::New();
    scalarThreshold->ThresholdBetween( scalar - 1e-5, scalar + 1e-5 );
    scalarThreshold->AllScalarsOn();
    scalarThreshold->SetInputConnection( contourFilter->GetOutputPort() );

    vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter =
      vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
    surfaceFilter->SetInputConnection( scalarThreshold->GetOutputPort() );

    vtkSmartPointer<vtkPolyDataConnectivityFilter> contourConnected =
      vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
    if ( firstCrossSection )
      {
      contourConnected->SetExtractionModeToAllRegions();
      firstCrossSection = false;
      }
    else
      {
      contourConnected->SetExtractionModeToClosestPointRegion();
      contourConnected->SetClosestPoint(previousCenterlinePoint);
      }
    contourConnected->SetInputConnection(surfaceFilter->GetOutputPort());
    contourConnected->Update();

    // Center of mass of surface elements is the average of the
    // centers of the surface triangles weighted by the triangle area.
    vtkPolyData* pd = contourConnected->GetOutput();

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

    if ( totalArea > 0.0 )
      {
      centerOfMass[0] /= totalArea;
      centerOfMass[1] /= totalArea;
      centerOfMass[2] /= totalArea;
      }
    else
      {
      centerOfMass[0] = centerOfMass[1] = centerOfMass[2] = 0.0;
      }

    if ( totalArea > 0.0 )
      {
      averageNormal[0] /= totalArea;
      averageNormal[1] /= totalArea;
      averageNormal[2] /= totalArea;
      }
    else
      {
      averageNormal[0] = averageNormal[1] = averageNormal[2] = 0.0;
      }

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

    vtkSmartPointer<vtkContourCompleter> completer =
      vtkSmartPointer<vtkContourCompleter>::New();
    completer->SetInputConnection( cutter->GetOutputPort() );

    vtkSmartPointer<vtkContourTriangulator> triangulate =
      vtkSmartPointer<vtkContourTriangulator>::New();
    triangulate->TriangulationErrorDisplayOn();
    triangulate->SetInputConnection( completer->GetOutputPort() );

    vtkSmartPointer<vtkPolyDataConnectivityFilter> connected =
      vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
    connected->SetExtractionModeToClosestPointRegion();
    connected->SetClosestPoint( previousCenterlinePoint );
    connected->SetInputConnection( triangulate->GetOutputPort() );
    connected->Update();

    vtkIdType numCells = connected->GetOutput()->GetNumberOfPolys();
    if ( numCells == 0 )
      {
      double zero = 0.0;
      areaInfo->SetTupleValue( contourID, &zero );
      perimeterInfo->SetTupleValue( contourID, &zero );
      continue;
      }

    appender->AddInputConnection( connected->GetOutputPort() );
    connected->Update();
    pd = connected->GetOutput();

    // Add heat values to point array
    for ( vtkIdType ptId = 0; ptId < pd->GetNumberOfPoints(); ++ptId )
      {
      heatValues->InsertNextTupleValue( &scalar );
      }

    // Add contour ID to cell array
    for ( vtkIdType cellId = 0; cellId < pd->GetNumberOfCells(); ++cellId )
      {
      contourIDs->InsertNextTupleValue( &contourID );
      }

    // Now measure the surface area of the planar cross section
    totalArea = 0.0;
    centerOfMass[0] = centerOfMass[1] = centerOfMass[2] = 0.0;
    averageNormal[0] = averageNormal[1] = averageNormal[2] = 0.0;
    ca = pd->GetPolys();
    ca->InitTraversal();
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

    if ( totalArea > 0.0 )
      {
      centerOfMass[0] /= totalArea;
      centerOfMass[1] /= totalArea;
      centerOfMass[2] /= totalArea;

      previousCenterlinePoint[0] = centerOfMass[0];
      previousCenterlinePoint[1] = centerOfMass[1];
      previousCenterlinePoint[2] = centerOfMass[2];
      }
    else
      {
      centerOfMass[0] = centerOfMass[1] = centerOfMass[2] = 0.0;
      }

    if ( totalArea > 0.0 )
      {
      averageNormal[0] /= totalArea;
      averageNormal[1] /= totalArea;
      averageNormal[2] /= totalArea;
      }
    else
      {
      averageNormal[0] = averageNormal[1] = averageNormal[2] = 0.0;
      }

    vtkSmartPointer<vtkMassProperties> massProperties =
      vtkSmartPointer<vtkMassProperties>::New();
    massProperties->SetInputConnection( connected->GetOutputPort() );
    massProperties->Update();
    double surfaceArea = massProperties->GetSurfaceArea();

    // Now compute the perimeter of this planar cross section.
    double perimeter = 0.0;
    vtkSmartPointer<vtkFeatureEdges> edges = vtkSmartPointer<vtkFeatureEdges>::New();
    edges->BoundaryEdgesOn();
    edges->FeatureEdgesOff();
    edges->NonManifoldEdgesOff();
    edges->ManifoldEdgesOff();
    edges->ColoringOff();
    edges->SetInputConnection( connected->GetOutputPort() );
    edges->Update();

    vtkPolyData* edgeOutput = edges->GetOutput();
    for ( vtkIdType cellId = 0; cellId < edgeOutput->GetNumberOfCells(); ++cellId )
      {
      vtkCell* cell = edgeOutput->GetCell( cellId );
      vtkLine* line = vtkLine::SafeDownCast( cell );
      vtkPolyLine* polyLine = vtkPolyLine::SafeDownCast( cell );
      if ( line )
        {
        vtkIdType ptId0 = line->GetPointId( 0 );
        vtkIdType ptId1 = line->GetPointId( 1 );

        double pt0[3], pt1[3];
        edgeOutput->GetPoint( ptId0, pt0 );
        edgeOutput->GetPoint( ptId1, pt1 );

        perimeter += sqrt( vtkMath::Distance2BetweenPoints( pt0, pt1 ) );
        }

      if ( polyLine )
        {
        vtkIdList* pointIds = polyLine->GetPointIds();
        int numPolyLinePoints = pointIds->GetNumberOfIds();
        if ( numPolyLinePoints > 1 )
          {
          for (vtkIdType i = 0; i < numPolyLinePoints; ++i)
            {
            vtkIdType ptId0 = pointIds->GetId( i );
            vtkIdType ptId1 = pointIds->GetId( (i + 1) % numPolyLinePoints );
            double pt0[3], pt1[3];
            edgeOutput->GetPoints()->GetPoint( ptId0, pt0 );
            edgeOutput->GetPoints()->GetPoint( ptId1, pt1 );

            perimeter += sqrt( vtkMath::Distance2BetweenPoints( pt0, pt1 ) );
            }
          }
        }
      }

    areaInfo->SetTupleValue( contourID, &surfaceArea );
    perimeterInfo->SetTupleValue( contourID, &perimeter );
    centerOfMassInfo->SetTupleValue( contourID, centerOfMass );
    averageNormalInfo->SetTupleValue( contourID, averageNormal );
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

  vtkSmartPointer<vtkPointSet> outputCopy;
  outputCopy.TakeReference( transformFilter->GetOutput()->NewInstance() );
  outputCopy->ShallowCopy( transformFilter->GetOutput() );

  // Add the point data to the output
  vtkPointData* pointData = outputCopy->GetPointData();
  pointData->SetScalars( heatValues );

  // Add the cell data to the output
  vtkCellData* cellData = outputCopy->GetCellData();
  cellData->SetScalars( contourIDs );

  // Add the field data to the output
  vtkFieldData* fieldData = outputCopy->GetFieldData();
  fieldData->AddArray( centerOfMassInfo );
  fieldData->AddArray( averageNormalInfo );
  fieldData->AddArray( areaInfo );
  fieldData->AddArray( perimeterInfo );

  // Write contours
  vtkSmartPointer<vtkXMLPolyDataWriter> pdWriter =
    vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  pdWriter->SetFileName( outputCrossSections.c_str() );
  pdWriter->SetInputData( outputCopy );
  pdWriter->Write();

  // Write CSV file
  vtkSmartPointer<vtkTable> table = vtkSmartPointer<vtkTable>::New();
  table->AddColumn( centerOfMassInfo );
  table->AddColumn( averageNormalInfo );
  table->AddColumn( areaInfo );
  table->AddColumn( perimeterInfo );

  vtkSmartPointer<vtkDelimitedTextWriter> tableWriter =
    vtkSmartPointer<vtkDelimitedTextWriter>::New();
  tableWriter->SetInputData( table );
  tableWriter->SetFileName( outputCSVFile.c_str() );
  tableWriter->Write();

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
