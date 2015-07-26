#ifndef vtkContourCompleter_h
#define vtkContourCompleter_h

#include <vtkPolyDataAlgorithm.h>

// Description:
// Connects nearly complete contours consisting of line segments.  If
// the contour is connected, nothing happens. Only a single line
// segment will be added, so contours with more than one missing line
// segment will continue to be incomplete.
class vtkContourCompleter : public vtkPolyDataAlgorithm
{
public:
  static vtkContourCompleter *New();
  vtkTypeMacro(vtkContourCompleter, vtkPolyDataAlgorithm);
  virtual void PrintSelf(ostream &os, vtkIndent indent);

protected:
  vtkContourCompleter();
  ~vtkContourCompleter();

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*);

private:
  vtkContourCompleter(const vtkContourCompleter&); // Not implemented
  void operator=(const vtkContourCompleter&); // Not implemented

};

#endif // vtkContourCompleter_h

