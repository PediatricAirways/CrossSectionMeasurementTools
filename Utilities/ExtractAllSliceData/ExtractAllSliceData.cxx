#include <vtkXMLPolyDataReader.h>
#include <vtkSmartPointer.h>
#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkPolyData.h>
#include <iostream>
#include <fstream>


int main ( int argc, char *argv[] )
{
  // Parse command line arguments
  if(argc < 3)
    {
    std::cerr << "Usage: " << argv[0]
              << " Filename(.vtp) outputfilename" << std::endl;
    return EXIT_FAILURE;
    }
 
  std::string filename = argv[1];
  std::string outname  = argv[2];

  std::ofstream myfile;
  myfile.open(outname.c_str());
  
 
  // Read all the data from the file
  vtkSmartPointer<vtkXMLPolyDataReader> reader =
    vtkSmartPointer<vtkXMLPolyDataReader>::New();
  reader->SetFileName(filename.c_str());
  reader->Update();

 vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
 polydata = reader->GetOutput();

 vtkSmartPointer<vtkDoubleArray> area = vtkSmartPointer<vtkDoubleArray>::New();

 for ( int i = 0; i < polydata->GetFieldData()->GetArray("area")->GetSize(); ++i )
 {
   myfile << polydata->GetFieldData()->GetArray("area")->GetTuple(i)[0]<< ", " <<
                polydata->GetFieldData()->GetArray("perimeter")->GetTuple(i)[0]<<  ", " << 
                polydata->GetFieldData()->GetArray("center of mass")->GetTuple(i)[0]<< " " << 
                polydata->GetFieldData()->GetArray("center of mass")->GetTuple(i)[1]<< " " << 
                polydata->GetFieldData()->GetArray("center of mass")->GetTuple(i)[2]<<std::endl;

 }
 myfile.close();
 
  return EXIT_SUCCESS;
}
