#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkXMLPolyDataReader.h>

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

  // Find TVC
  vtkFieldData* fd = polydata->GetFieldData();
  vtkStringArray* names = vtkStringArray::SafeDownCast(fd->GetAbstractArray("query point name"));
  if (!names)
    {
    std::cerr << "Could not find 'query point name' string array in input\n";
    return EXIT_FAILURE;
    }

  vtkIdType tvcID                = names->LookupValue( "TVC" );
  vtkIdType inferiorSubglottisID = names->LookupValue( "InferiorSubglottis" );
  vtkIdType tracheaCarinaID      = names->LookupValue( "TracheaCarina" );

  // TVC
  if ( tvcID >= 0 )
    {
    myfile << fd->GetArray("contour ID")->GetTuple(tvcID)[0] << std::endl;
    }
  else
    {
    myfile << -1 << std::endl;
    }

  // Inferior subglottis
  if ( inferiorSubglottisID >= 0 )
    {
    myfile << polydata->GetFieldData()->GetArray("contour ID")->GetTuple(inferiorSubglottisID)[0] << std::endl;
    }
  else
    {
    myfile << -1 << std::endl;
    }

  // Trachea carina
  if ( tracheaCarinaID )
    {
    myfile << polydata->GetFieldData()->GetArray("contour ID")->GetTuple(tracheaCarinaID)[0] << std::endl;
    }
  else
    {
    myfile << -1 << std::endl;
    }

  myfile.close();
 
  return EXIT_SUCCESS;
}
