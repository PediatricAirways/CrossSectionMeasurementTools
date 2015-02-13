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

  ofstream myfile;
  myfile.open(outname);
  
 
  // Read all the data from the file
  vtkSmartPointer<vtkXMLPolyDataReader> reader =
    vtkSmartPointer<vtkXMLPolyDataReader>::New();
  reader->SetFileName(filename.c_str());
  reader->Update();

  vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
  polydata = reader->GetOutput();

  vtkSmartPointer<vtkDoubleArray> area = vtkSmartPointer<vtkDoubleArray>::New();


  myfile << polydata->GetFieldData()->GetArray("contour ID")->GetTuple(0)[0]<<std::endl; //TVC
  myfile << polydata->GetFieldData()->GetArray("contour ID")->GetTuple(2)[0]<<std::endl; //Inferrior Subglottis
  myfile << polydata->GetFieldData()->GetArray("contour ID")->GetTuple(3)[0]<<std::endl; //Trachea Carina
  myfile.close();
 
  return EXIT_SUCCESS;
}