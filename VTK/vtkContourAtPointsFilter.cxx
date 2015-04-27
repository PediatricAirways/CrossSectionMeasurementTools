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
  vtkInformation* inDataSetInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* inPointsInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  vtkDataSet* input = vtkDataSet::SafeDownCast(
    inDataSetInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPointSet* inPoints = vtkPointSet::SafeDownCast(
    inPointsInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkDataSet* output = vtkDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkSmartPointer<vtkProbeFilter> probe = vtkSmartPointer<vtkProbeFilter>::New();
  probe->SetInputData(inPoints);
  probe->SetSourceData(input);
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

  // Get the point data at the points and use these values as
  // isosurface values for the heat flow image.
  vtkIdType numberOfPoints = pointSet->GetNumberOfPoints();

  vtkSmartPointer<vtkContourFilter> contourFilter =
    vtkSmartPointer<vtkContourFilter>::New();
  contourFilter->SetNumberOfContours(numberOfPoints);
  contourFilter->ComputeNormalsOn();
  contourFilter->ComputeScalarsOn();
  contourFilter->GenerateTrianglesOn();
  contourFilter->SetInputData(input);

  vtkDataArray* da = pointSet->GetPointData()->GetScalars();
  for (int i = 0; i < pointSet->GetNumberOfPoints(); ++i)
    {
    contourFilter->SetValue(i, da->GetTuple1(i));
    std::cout << "setting value: " << da->GetTuple1(i) << std::endl;
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
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
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
