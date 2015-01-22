#include "vtkCrossSectionImageFilter.h"

#include "vtkAlgorithm.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkCrossSectionImageFilter);

//----------------------------------------------------------------------------
vtkCrossSectionImageFilter::vtkCrossSectionImageFilter()
{
}

//----------------------------------------------------------------------------
vtkCrossSectionImageFilter::~vtkCrossSectionImageFilter()
{
}

//----------------------------------------------------------------------------
int vtkCrossSectionImageFilter::RequestData(
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
  vtkPolyData* output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  return 1;
}

//----------------------------------------------------------------------------
int vtkCrossSectionImageFilter::FillInputPortInformation(int port, vtkInformation *info)
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
void vtkCrossSectionImageFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
