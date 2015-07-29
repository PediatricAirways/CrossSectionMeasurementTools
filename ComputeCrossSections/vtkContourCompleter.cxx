#include "vtkContourCompleter.h"

#include <vtkAppendPolyData.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkIdList.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkSmartPointer.h>
#include <vtkStripper.h>
#include <vtkThreshold.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkContourCompleter);

//----------------------------------------------------------------------------
vtkContourCompleter::vtkContourCompleter()
{
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfInputPorts(1);
}

//----------------------------------------------------------------------------
vtkContourCompleter::~vtkContourCompleter()
{
}

//----------------------------------------------------------------------------
int vtkContourCompleter::RequestData(vtkInformation*        vtkNotUsed(request),
                                     vtkInformationVector** inputVector,
                                     vtkInformationVector*  outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  vtkPolyData* inputPD = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPolyData* outputPD = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // Find the connected components
  vtkSmartPointer<vtkPolyDataConnectivityFilter> connectivity =
    vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
  connectivity->SetExtractionModeToAllRegions();
  connectivity->ColorRegionsOn();
  connectivity->SetInputData( inputPD );
  connectivity->Update();

  int numberOfRegions = connectivity->GetNumberOfExtractedRegions();

  vtkSmartPointer<vtkAppendPolyData> appender = vtkSmartPointer<vtkAppendPolyData>::New();

  // Iterate over the connected components, complete the contours if needed,
  // then append it all back together
  for (vtkIdType regionId = 0; regionId < numberOfRegions; ++regionId)
    {
    vtkSmartPointer<vtkThreshold> scalarThreshold = vtkSmartPointer<vtkThreshold>::New();
    scalarThreshold->ThresholdBetween( regionId, regionId );
    scalarThreshold->SetInputArrayToProcess(0, 0, 0,
                                            vtkDataObject::FIELD_ASSOCIATION_POINTS,
                                            "RegionId");
    scalarThreshold->SetInputConnection( connectivity->GetOutputPort() );

    vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter =
      vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
    surfaceFilter->SetInputConnection( scalarThreshold->GetOutputPort() );

    vtkSmartPointer<vtkStripper> stripper =
      vtkSmartPointer<vtkStripper>::New();
    stripper->JoinContiguousSegmentsOn();
    stripper->SetInputConnection( surfaceFilter->GetOutputPort() );
    stripper->Update();

    vtkSmartPointer<vtkPolyData> connectedPD = vtkSmartPointer<vtkPolyData>::New();
    connectedPD->DeepCopy( stripper->GetOutput() );

    vtkSmartPointer<vtkIdList> pts = vtkSmartPointer<vtkIdList>::New();
    connectedPD->GetCellPoints(0, pts);

    vtkIdType firstPt = pts->GetId(0);
    vtkIdType lastPt = pts->GetId( pts->GetNumberOfIds()-1 );
    if (firstPt != lastPt)
      {
      // Insert the first point as the last to complete the contour
      pts->InsertNextId(firstPt);
      }

    connectedPD->ReplaceCell( 0, pts->GetNumberOfIds(), pts->GetPointer(0) );
    
    appender->AddInputData(connectedPD);
    }

  appender->Update();

  outputPD->DeepCopy(appender->GetOutput());

  return 1;
}


//----------------------------------------------------------------------------
void vtkContourCompleter::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
