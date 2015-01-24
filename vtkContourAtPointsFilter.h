#ifndef vtkCrossSectionImageFilter_h_included
#define vtkCrossSectionImageFilter_h_included

#include <vtkPolyDataAlgorithm.h>

// .NAME vtkCrossSectionImageFilter - very specific-purpose filter for
// computing cross sections that pass approximately through a set of
// input points. The filter requires two inputs, an image that
// contains the solution to the Laplace equation for heat flow through
// a cylinder-like object and a set of points through which the cross
// sections should pass. The output is a poly data consisting of cross
// sections through the input points.
class vtkCrossSectionImageFilter : public vtkPolyDataAlgorithm
{
public:
  static vtkCrossSectionImageFilter *New();
  vtkTypeMacro(vtkCrossSectionImageFilter,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkCrossSectionImageFilter();
  ~vtkCrossSectionImageFilter();

  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int port, vtkInformation *info);

private:
  vtkCrossSectionImageFilter(const vtkCrossSectionImageFilter&); // Not implemented.
  void operator=(const vtkCrossSectionImageFilter&); // Not implemented
};

#endif
