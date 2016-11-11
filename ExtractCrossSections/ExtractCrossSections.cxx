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
#include "ExtractCrossSectionsCLP.h"

#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCellLocator.h>
#include <vtkContourFilter.h>
#include <vtkCutter.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkDelimitedTextWriter.h>
#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkGenericCell.h>
#include <vtkMassProperties.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
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

/*******************************************************************/
int main( int argc, char* argv[] )
{
  PARSE_ARGS;

  int returnValue = EXIT_SUCCESS;

  vtkSmartPointer<vtkXMLPolyDataReader> crossSectionsReader =
    vtkSmartPointer<vtkXMLPolyDataReader>::New();
  crossSectionsReader->SetFileName( crossSections.c_str() );
  crossSectionsReader->Update();

  vtkPolyData* crossSectionsPD = crossSectionsReader->GetOutput();

  vtkFieldData* inputFieldData = crossSectionsPD->GetFieldData();

  vtkDoubleArray* inputCenterOfMassInfo =
    vtkDoubleArray::SafeDownCast( inputFieldData->GetArray( "center of mass" ) );
  if ( !inputCenterOfMassInfo )
    {
    std::cerr << "Input center of mass field data array is missing and won't be available in the output.\n";
    }

  vtkDoubleArray* inputAverageNormalInfo =
    vtkDoubleArray::SafeDownCast( inputFieldData->GetArray( "normal" ) );
  if ( !inputAverageNormalInfo )
    {
    std::cerr << "Input average normal field data array is missing and won't be available in the output.\n";
    }

  vtkDoubleArray* inputAreaInfo =
    vtkDoubleArray::SafeDownCast( inputFieldData->GetArray( "area" ) );
  if ( !inputAreaInfo )
    {
    std::cerr << "Input area field data array is missing and won't be available in the output.\n";
    }

  vtkDoubleArray* inputPerimeterInfo =
    vtkDoubleArray::SafeDownCast( inputFieldData->GetArray( "perimeter" ) );
  if ( !inputPerimeterInfo )
    {
    std::cerr << "Input perimeter field data array is missing and won't be available in the output.\n";
    }

  // For each query point, find nearest cross section
  vtkSmartPointer<vtkCellLocator> cellLocator =
    vtkSmartPointer<vtkCellLocator>::New();
  cellLocator->SetDataSet( crossSectionsPD );
  cellLocator->BuildLocator();

  // Field data containing meta data about the cross sections. One
  // entry for each cross-section is stored for each of the arrays
  // centerOfMassInfo, averageNormalInfo, areaInfo, and perimeterInfo.
  vtkSmartPointer<vtkIdTypeArray> queryPtIDInfo = vtkSmartPointer<vtkIdTypeArray>::New();
  queryPtIDInfo->SetName( "query point ID" );
  queryPtIDInfo->SetNumberOfComponents( 1 );

  // Add query point names associated with cross sections
  vtkSmartPointer<vtkStringArray> queryPtNameInfo = vtkSmartPointer<vtkStringArray>::New();
  queryPtNameInfo->SetName( "query point name" );
  queryPtNameInfo->SetNumberOfComponents(1);

  vtkSmartPointer<vtkIdTypeArray> contourIDInfo = vtkSmartPointer<vtkIdTypeArray>::New();
  contourIDInfo->SetName( "contour ID" );
  contourIDInfo->SetNumberOfComponents( 1 );

  vtkSmartPointer<vtkDoubleArray> centerOfMassInfo = vtkSmartPointer<vtkDoubleArray>::New();
  centerOfMassInfo->SetName( "center of mass" );
  centerOfMassInfo->SetNumberOfComponents( 3 );

  vtkSmartPointer<vtkDoubleArray> averageNormalInfo = vtkSmartPointer<vtkDoubleArray>::New();
  averageNormalInfo->SetName( "normal" );
  averageNormalInfo->SetNumberOfComponents( 3 );

  vtkSmartPointer<vtkDoubleArray> areaInfo = vtkSmartPointer<vtkDoubleArray>::New();
  areaInfo->SetName( "area" );
  areaInfo->SetNumberOfComponents( 1 );

  vtkSmartPointer<vtkDoubleArray> perimeterInfo = vtkSmartPointer<vtkDoubleArray>::New();
  perimeterInfo->SetName( "perimeter" );
  perimeterInfo->SetNumberOfComponents( 1 );

  vtkSmartPointer<vtkAppendPolyData> appender =
    vtkSmartPointer<vtkAppendPolyData>::New();

  for ( size_t inputPtID = 0; inputPtID < queryPoints.size(); ++inputPtID )
    {
    double queryPoint[3], closestPoint[3];
    queryPoint[0] = queryPoints[inputPtID][0];
    queryPoint[1] = queryPoints[inputPtID][1];
    queryPoint[2] = queryPoints[inputPtID][2];
    vtkIdType cellID;
    int subId;
    double dist2;
    cellLocator->FindClosestPoint( queryPoint, closestPoint, cellID, subId, dist2 );

    double dist2Threshold = 2.0; // mm
    dist2Threshold *= dist2Threshold;
    if ( cellID < 0 || dist2 > dist2Threshold )
      {
      std::cout << "For query point " << queryPointNames[ inputPtID ]
                << ", nearest point dist^2 is " << dist2 << std::endl;
      continue;
      }

    vtkDataArray* scalars = crossSectionsPD->GetCellData()->GetScalars();
    double nearestScalar = scalars->GetTuple1( cellID );

    // Extract contour for the nearest scalar value
    vtkSmartPointer<vtkThreshold> scalarThreshold = vtkSmartPointer<vtkThreshold>::New();
    scalarThreshold->ThresholdBetween( nearestScalar - 1e-5, nearestScalar + 1e-5 );
    scalarThreshold->AllScalarsOff();
    scalarThreshold->SetInputArrayToProcess( 0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS,
                                             "contour ID" );
    scalarThreshold->SetInputConnection( crossSectionsReader->GetOutputPort() );
    scalarThreshold->Update();

    vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter =
      vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
    surfaceFilter->SetInputConnection( scalarThreshold->GetOutputPort() );

    appender->AddInputConnection( surfaceFilter->GetOutputPort() );

    // Add field data entries
    vtkIdType queryPtID = static_cast<vtkIdType>( inputPtID );
    queryPtIDInfo->InsertNextTypedTuple( &queryPtID );

    std::string queryPtName = queryPointNames[ inputPtID ];
    queryPtNameInfo->InsertNextValue( queryPtName.c_str() );

    vtkIdType contourID = static_cast<vtkIdType>( nearestScalar );
    contourIDInfo->InsertNextTypedTuple( &contourID );

    double tuple[3];
    if ( inputCenterOfMassInfo )
      {
      inputCenterOfMassInfo->GetTypedTuple( contourID, tuple );
      centerOfMassInfo->InsertNextTypedTuple( tuple );
      }

    if ( inputAverageNormalInfo )
      {
      inputAverageNormalInfo->GetTypedTuple( contourID, tuple );
      averageNormalInfo->InsertNextTypedTuple( tuple );
      }

    if ( inputAreaInfo )
      {
      inputAreaInfo->GetTypedTuple( contourID, tuple );
      areaInfo->InsertNextTypedTuple( tuple );
      }

    if ( inputAreaInfo )
      {
      inputPerimeterInfo->GetTypedTuple( contourID, tuple );
      perimeterInfo->InsertNextTypedTuple( tuple );
      }
    }

  appender->Update();
  vtkSmartPointer<vtkPointSet> outputCopy;
  outputCopy.TakeReference( appender->GetOutput()->NewInstance() );
  outputCopy->ShallowCopy( appender->GetOutput() );

  // Add our field data
  vtkSmartPointer<vtkFieldData> fieldData = vtkSmartPointer<vtkFieldData>::New();
  fieldData->AddArray( queryPtIDInfo );
  fieldData->AddArray( queryPtNameInfo );
  fieldData->AddArray( contourIDInfo );
  fieldData->AddArray( centerOfMassInfo );
  fieldData->AddArray( averageNormalInfo );
  fieldData->AddArray( areaInfo );
  fieldData->AddArray( perimeterInfo );

  outputCopy->SetFieldData( fieldData );

  // Write contours
  vtkSmartPointer<vtkXMLPolyDataWriter> pdWriter =
    vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  pdWriter->SetFileName( extractedCrossSectionGeometry.c_str() );
  pdWriter->SetInputData( outputCopy );
  pdWriter->Write();

  // Write CSV file
  vtkSmartPointer<vtkTable> table = vtkSmartPointer<vtkTable>::New();
  table->AddColumn( queryPtNameInfo );
  table->AddColumn( queryPtIDInfo );
  table->AddColumn( centerOfMassInfo );
  table->AddColumn( averageNormalInfo );
  table->AddColumn( areaInfo );
  table->AddColumn( perimeterInfo );

  vtkSmartPointer<vtkDelimitedTextWriter> tableWriter =
    vtkSmartPointer<vtkDelimitedTextWriter>::New();
  tableWriter->SetInputData( table );
  tableWriter->SetFileName( extractedCrossSectionCSV.c_str() );
  tableWriter->Write();

  return returnValue;
}
