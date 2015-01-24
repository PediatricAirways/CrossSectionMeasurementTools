#include "vtkContourAtPointsFilter.h"

#include "vtkAlgorithm.h"
#include "vtkContourFilter.h"
#include "vtkDataSet.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointSet.h"
#include "vtkProbeFilter.h"
#include "vtkSmartPointer.h"
#include "vtkThreshold.h"


vtkStandardNewMacro(vtkContourAtPointsFilter);

//----------------------------------------------------------------------------
vtkContourAtPointsFilter::vtkContourAtPointsFilter()
{
  this->SetNumberOfInputPorts(2);
}

//----------------------------------------------------------------------------
vtkContourAtPointsFilter::~vtkContourAtPointsFilter()
{
}

//----------------------------------------------------------------------------
int vtkContourAtPointsFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  vtkInformation* inImageInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* inPointsInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  vtkImageData* inImage = vtkImageData::SafeDownCast(
    inImageInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPointSet* inPoints = vtkPointSet::SafeDownCast(
    inPointsInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkDataSet* output = vtkDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkSmartPointer<vtkProbeFilter> probe = vtkSmartPointer<vtkProbeFilter>::New();
  probe->SetInputData(inPoints);
  probe->SetSourceData(inImage);
  probe->Update();

  // Get the probe values from the output
  vtkDataSet* probeOutput = probe->GetOutput();
  if (!probeOutput)
    {
    vtkErrorMacro(<< "No output from probe filter");
    return 0;
    }

  vtkPointSet* pointSet = vtkPointSet::SafeDownCast(probeOutput);
  if (!pointSet)
    {
    vtkErrorMacro(<< "Probe filter did not return a vtkPointSet");
    return 0;
    }

  // Threshold the image file to get rid of the NaNs
  vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
  threshold->ThresholdBetween(0.0, 1.0);
  threshold->AllScalarsOn();
  threshold->SetInputData(inImage);

  // Get the point data at the points and use these values as
  // isosurface values for the heat flow image.
  vtkIdType numberOfPoints = pointSet->GetNumberOfPoints();

  vtkSmartPointer<vtkContourFilter> contourFilter =
    vtkSmartPointer<vtkContourFilter>::New();
  contourFilter->SetNumberOfContours(numberOfPoints);
  contourFilter->ComputeNormalsOn();
  contourFilter->GenerateTrianglesOn();
  contourFilter->SetInputConnection(threshold->GetOutputPort());

  vtkDataArray* da = pointSet->GetPointData()->GetScalars();
  for (int i = 0; i < pointSet->GetNumberOfPoints(); ++i)
    {
    contourFilter->SetValue(i, da->GetTuple1(i));
    }

  // Create a contour filter and set the isovalues
  contourFilter->Update();

  // Copy output of contour filter to output
  output->ShallowCopy(contourFilter->GetOutput());

  return 1;
}

//----------------------------------------------------------------------------
int vtkContourAtPointsFilter::FillInputPortInformation(int port, vtkInformation *info)
{
  if (port == 0)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
    return 1;
    }
  else if (port == 1)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPointSet");
    return 1;
    }

  return 0;
}

//----------------------------------------------------------------------------
void vtkContourAtPointsFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
