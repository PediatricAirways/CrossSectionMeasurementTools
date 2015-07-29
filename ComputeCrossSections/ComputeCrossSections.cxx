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
#include <vtkFloatArray.h>
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

#include <set>

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
    std::cout << "heat flow contours               = " << heatFlowContours << std::endl;
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
int main( int argc, char* argv[] )
{
  PARSE_ARGS;

  int returnValue = EXIT_SUCCESS;

  vtkSmartPointer<vtkXMLPolyDataReader> contourReader =
    vtkSmartPointer<vtkXMLPolyDataReader>::New();
  contourReader->SetFileName( heatFlowContours.c_str() );
  contourReader->Update();
  vtkPointData* contourPD = contourReader->GetOutput()->GetPointData();
  vtkFloatArray* heatArray = vtkFloatArray::SafeDownCast(contourPD->GetArray("scalars"));
  if (!heatArray)
    {
    std::cerr << "'scalars' point data array not available in contours file '"
              << heatFlowContours << "'\n";
    return EXIT_FAILURE;
    }

  // Create the set of heat values in the contours
  std::set<float> contourValueSet;
  for ( vtkIdType id = 0; id < heatArray->GetNumberOfTuples(); ++id )
    {
    float value = heatArray->GetValue( id );
    contourValueSet.insert( value );
    }
  std::vector<float> contourValues( contourValueSet.begin(), contourValueSet.end() );

  int numContours = static_cast<int>( contourValues.size() );
  std::cout << "Num contours: " << numContours << std::endl;

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

  vtkSmartPointer<vtkAppendPolyData> appendCuts =
    vtkSmartPointer<vtkAppendPolyData>::New();

  bool firstCrossSection = true;
  double previousCenterlinePoint[3] = {0.0, 0.0, 0.0};

  for ( vtkIdType contourID = numContours-1; contourID >= 0; --contourID )
    //for ( vtkIdType contourID = 450; contourID >= 420; --contourID )
    {
    //double scalar = contourFilter->GetValue( contourID );
    double scalar = contourValues[contourID];

    std::cout << "Processing contour " << contourID << " - " << scalar <<std::endl;

    // Extract contour for the nearest scalar value
    vtkSmartPointer<vtkThreshold> scalarThreshold = vtkSmartPointer<vtkThreshold>::New();
    scalarThreshold->ThresholdBetween( scalar - 1e-5, scalar + 1e-5 );
    scalarThreshold->AllScalarsOn();
    scalarThreshold->SetInputConnection( contourReader->GetOutputPort() );

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
    completer->Update();

    vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    char fileNameBuffer[1024];
    sprintf(fileNameBuffer, "%s-%04lld.vtp", outputCrossSections.c_str(), contourID);
    writer->SetFileName( fileNameBuffer );
    writer->SetInputConnection( completer->GetOutputPort() );
    writer->Write();

    vtkSmartPointer<vtkContourTriangulator> triangulate =
      vtkSmartPointer<vtkContourTriangulator>::New();
    triangulate->TriangulationErrorDisplayOn();
    triangulate->SetInputConnection( completer->GetOutputPort() );

    vtkSmartPointer<vtkPolyDataConnectivityFilter> connected =
      vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
    connected->SetExtractionModeToClosestPointRegion();
    connected->SetClosestPoint( previousCenterlinePoint );
    connected->SetInputConnection( triangulate->GetOutputPort() );
    //connected->SetInputConnection( completer->GetOutputPort() );
    connected->Update();

    appendCuts->AddInputConnection( completer->GetOutputPort() );

    vtkIdType numCells = connected->GetOutput()->GetNumberOfPolys();
    if ( numCells == 0 )
      {
      double zero = 0.0;
      areaInfo->SetTupleValue( contourID, &zero );
      perimeterInfo->SetTupleValue( contourID, &zero );

      // Assume contourID + 1 is valid...
      centerOfMassInfo->GetTupleValue( contourID + 1, centerOfMass );
      centerOfMassInfo->SetTupleValue( contourID, centerOfMass );
      averageNormalInfo->GetTupleValue( contourID + 1, averageNormal );
      averageNormalInfo->SetTupleValue( contourID, averageNormal );

      continue;
      }

    appendCuts->AddInputConnection( completer->GetOutputPort() );

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

    std::cout << " - area: " << totalArea << std::endl << std::flush;

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

  transformFilter->SetInputConnection( appendCuts->GetOutputPort() );
  transformFilter->Update();

  pdWriter->SetFileName( (outputCrossSections + "-cuts.vtp").c_str() );
  pdWriter->SetInputConnection( transformFilter->GetOutputPort() );
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
