#include <iostream>

#include "vtkCrossSectionImageFilter.h"

#include <vtkImageThreshold.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLPolyDataWriter.h>


int main(int argc, char* argv[])
{
  if (argc < 3)
    {
    std::cerr << "Usage: " << argv[0] << " <image file> <output polydata file>\n";
    return EXIT_FAILURE;
    }

  std::string inputImageFile(argv[1]);
  std::string outputPolyDataFile(argv[2]);

  // Load image file
  vtkSmartPointer<vtkXMLImageDataReader> reader =
    vtkSmartPointer<vtkXMLImageDataReader>::New();
  reader->SetFileName( argv[1] );

  // Set up cross section sample points
  vtkSmartPointer<vtkPoints> samplePoints = vtkSmartPointer<vtkPoints>::New();
  samplePoints->SetNumberOfPoints( 2 );
  samplePoints->SetPoint( 0, 3, 170, -166 );
  samplePoints->SetPoint( 1, 8.095, 178.0, -172.7 );
  //samplePoints->SetPoint( 2, 0, 0, 0 );

  vtkSmartPointer<vtkPolyData> pointSet = vtkSmartPointer<vtkPolyData>::New();
  pointSet->SetPoints( samplePoints );

  vtkSmartPointer<vtkCrossSectionImageFilter> crossSectionFilter =
    vtkSmartPointer<vtkCrossSectionImageFilter>::New();
  crossSectionFilter->SetInputConnection( 0, reader->GetOutputPort() );
  crossSectionFilter->SetInputDataObject( 1, pointSet );

  vtkSmartPointer<vtkXMLPolyDataWriter> polyDataWriter =
    vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  polyDataWriter->SetFileName( argv[2] );
  polyDataWriter->SetInputConnection( crossSectionFilter->GetOutputPort() );
  polyDataWriter->Update();

  return EXIT_SUCCESS;
}
