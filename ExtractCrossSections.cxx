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

#include <itkImageIOBase.h>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageToVTKImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkResampleImageFilter.h>
#include <itkSpatialOrientationAdapter.h>

#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCellLocator.h>
#include <vtkContourFilter.h>
#include <vtkCutter.h>
#include <vtkDataSetSurfaceFilter.h>
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
#include <vtkStripper.h>
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

  // For each query point, find nearest cross section
  vtkSmartPointer<vtkCellLocator> cellLocator =
    vtkSmartPointer<vtkCellLocator>::New();
  cellLocator->SetDataSet( crossSectionsReader->GetOutput() );
  cellLocator->BuildLocator();

  size_t numQueryPoints = queryPoints.size();

  vtkSmartPointer<vtkAppendPolyData> appender =
    vtkSmartPointer<vtkAppendPolyData>::New();

  for ( size_t inputPtId = 0; inputPtId < queryPoints.size(); ++inputPtId )
    {
    double queryPoint[3], closestPoint[3];
    queryPoint[0] = queryPoints[inputPtId][0];
    queryPoint[1] = queryPoints[inputPtId][1];
    queryPoint[2] = queryPoints[inputPtId][2];
    vtkIdType cellID;
    int subId;
    double dist2;
    cellLocator->FindClosestPoint( queryPoint, closestPoint, cellID, subId, dist2 );

    double dist2Threshold = 5.0 * 5.0; // mm. This might be too high.
    if ( cellID < 0 || dist2 > dist2Threshold )
      {
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
    }

  vtkSmartPointer<vtkXMLPolyDataWriter> pdWriter =
    vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  pdWriter->SetFileName( extractedCrossSections.c_str() );
  pdWriter->SetInputConnection( appender->GetOutputPort() );
  pdWriter->Write();

  return returnValue;
}
