#include "vtkContourCompleter.h"

#include <vtkAppendPolyData.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkIdList.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkSmartPointer.h>
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
    surfaceFilter->Update();

    vtkSmartPointer<vtkPolyData> connectedPD = vtkSmartPointer<vtkPolyData>::New();
    connectedPD->DeepCopy(surfaceFilter->GetOutput());

    // Find up to two points that are not connected and connect them
    vtkIdType numPts = 0, pointIds[2] = { -1, -1 };
    vtkSmartPointer<vtkIdList> cellIds = vtkSmartPointer<vtkIdList>::New();
    for (int ptId = 0; ptId < connectedPD->GetNumberOfPoints(); ++ptId)
      {
      connectedPD->GetPointCells(ptId, cellIds);
      int numCells = cellIds->GetNumberOfIds();
      if (numCells == 1)
        {
        pointIds[numPts] = ptId;
        ++numPts;
        if (numPts == 2)
          {
          break;
          }
        }
      }

    if (numPts == 2)
      {
      // Connect the two points with 1 cell each.
      connectedPD->InsertNextCell(VTK_LINE, 2, pointIds);
      }

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
