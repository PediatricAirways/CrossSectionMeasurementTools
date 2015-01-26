#include <iostream>

#include "vtkContourAtPointsFilter.h"

#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkCutter.h>
#include <vtkIdList.h>
#include <vtkImageThreshold.h>
#include <vtkMassProperties.h>
#include <vtkPlane.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkSmartPointer.h>
#include <vtkThreshold.h>
#include <vtkTriangle.h>
#include <vtkTriangleFilter.h>
#include <vtkStripper.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>


int main(int argc, char* argv[])
{
  if (argc < 3)
    {
    std::cerr << "Usage: " << argv[0] << " <image file> <output polydata file>\n";
    return EXIT_FAILURE;
    }

  std::string inputImageFile(argv[1]);
  std::string inputPolyDataFile(argv[2]);
  std::string outputPolyDataFile(argv[3]);

  // Load image file
  vtkSmartPointer<vtkXMLImageDataReader> reader =
    vtkSmartPointer<vtkXMLImageDataReader>::New();
  reader->SetFileName( inputImageFile.c_str() );

  // Load surface file
  vtkSmartPointer<vtkXMLPolyDataReader> surfaceReader =
    vtkSmartPointer<vtkXMLPolyDataReader>::New();
  surfaceReader->SetFileName( inputPolyDataFile.c_str() );

  // Set up cross section sample points
  vtkSmartPointer<vtkPoints> samplePoints = vtkSmartPointer<vtkPoints>::New();
  samplePoints->SetNumberOfPoints( 2 );
  samplePoints->SetPoint( 0, 3, 170, -166 );
  samplePoints->SetPoint( 1, 8.095, 178.0, -172.7 );
  //samplePoints->SetPoint( 2, 0, 0, 0 );

  vtkSmartPointer<vtkPolyData> pointSet = vtkSmartPointer<vtkPolyData>::New();
  pointSet->SetPoints( samplePoints );

  // Threshold the image file to get rid of the NaNs
  vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
  threshold->ThresholdBetween(0.0, 1.0);
  threshold->AllScalarsOn();
  threshold->SetInputConnection( reader->GetOutputPort() );

  vtkSmartPointer<vtkContourAtPointsFilter> contourFilter =
    vtkSmartPointer<vtkContourAtPointsFilter>::New();
  contourFilter->SetInputConnection( 0, threshold->GetOutputPort() );
  contourFilter->SetInputDataObject( 1, pointSet );

  // Compute the areas of the cross sections
  vtkSmartPointer<vtkPolyDataConnectivityFilter> connected =
    vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
  connected->SetExtractionModeToAllRegions();
  connected->ColorRegionsOn();
  connected->SetInputConnection(contourFilter->GetOutputPort());

  vtkSmartPointer<vtkXMLPolyDataWriter> polyDataWriter =
    vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  polyDataWriter->SetFileName( outputPolyDataFile.c_str() );
  polyDataWriter->SetInputConnection( connected->GetOutputPort() );
  polyDataWriter->Update();

  // Append all planar cross sections
  vtkSmartPointer<vtkAppendPolyData> append =
    vtkSmartPointer<vtkAppendPolyData>::New();

  // Now we extract each cross section, compute the center of gravity,
  // and the average normal.
  for ( vtkIdType ptId = 0; ptId < samplePoints->GetNumberOfPoints(); ++ptId )
    {
    vtkSmartPointer< vtkPolyDataConnectivityFilter > contourConnected =
      vtkSmartPointer< vtkPolyDataConnectivityFilter >::New();
    contourConnected->SetExtractionModeToClosestPointRegion();
    double point[3];
    samplePoints->GetPoint( ptId, point );
    contourConnected->SetClosestPoint( point );
    contourConnected->SetInputConnection( contourFilter->GetOutputPort() );
    contourConnected->Update();
    vtkPolyData* pd = contourConnected->GetOutput();
    vtkCellArray* ca = pd->GetPolys();

    // Center of mass of surface elements is the average of the
    // centers of the surface triangles weighted by the triangle area.
    ca->InitTraversal();
    vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();
    vtkSmartPointer<vtkIdList> ptList = vtkSmartPointer<vtkIdList>::New();
    double centerOfMass[3] = { 0.0, 0.0, 0.0 };
    double averageNormal[3] = { 0.0, 0.0, 0.0 };
    double totalArea = 0.0;
    while ( ca->GetNextCell( ptList ) )
      {
      double p0[3], p1[3], p2[3];
      pd->GetPoint( ptList->GetId( 0 ), p0 );
      pd->GetPoint( ptList->GetId( 1 ), p1 );
      pd->GetPoint( ptList->GetId( 2 ), p2 );
      double area = vtkTriangle::TriangleArea( p0, p1, p2 );
      totalArea += area;

      double center[3], normal[3];
      vtkTriangle::TriangleCenter( p0, p1, p2, center );
      vtkTriangle::ComputeNormal( p0, p1, p2, normal );

      for ( int i = 0; i < 3; ++i )
        {
        centerOfMass[i]  += area * center[i];
        averageNormal[i] += area * normal[i];
        }
      }

    centerOfMass[0] /= totalArea;
    centerOfMass[1] /= totalArea;
    centerOfMass[2] /= totalArea;

    std::cout << "com: "
              << centerOfMass[0] << ", "
              << centerOfMass[1] << ", "
              << centerOfMass[2] << std::endl;

    averageNormal[0] /= totalArea;
    averageNormal[1] /= totalArea;
    averageNormal[2] /= totalArea;

    std::cout << "normal: "
              << averageNormal[0] << ", "
              << averageNormal[1] << ", "
              << averageNormal[2] << std::endl;

    // Now cut the polygonal model from the segmentation by the plane
    // defined by the center of mass and normal
    vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
    plane->SetOrigin( centerOfMass );
    plane->SetNormal( averageNormal );

    vtkSmartPointer<vtkCutter> cutter = vtkSmartPointer<vtkCutter>::New();
    cutter->SetCutFunction( plane );
    cutter->GenerateCutScalarsOn();
    cutter->SetNumberOfContours( 0 );
    cutter->SetValue( 0, 0.0 );
    cutter->SetInputConnection( surfaceReader->GetOutputPort() );
    cutter->Update();

    vtkSmartPointer<vtkPolyDataConnectivityFilter> planarCutConnectivity =
      vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
    planarCutConnectivity->SetExtractionModeToClosestPointRegion();
    planarCutConnectivity->SetClosestPoint( centerOfMass );
    planarCutConnectivity->SetInputConnection( cutter->GetOutputPort() );

    // Convert planar cut into line strips
    vtkSmartPointer<vtkStripper> stripper = vtkSmartPointer<vtkStripper>::New();
    stripper->SetInputConnection( planarCutConnectivity->GetOutputPort() );
    stripper->Update();

    // Convert the poly line from the cut to a polygon
    vtkPolyData* cut = stripper->GetOutput();

    vtkCell* polyLine = cut->GetCell( 0 ); // Assumes one cut object
    vtkSmartPointer<vtkPolyData> polygon = vtkSmartPointer<vtkPolyData>::New();
    polygon->Allocate();
    polygon->SetPoints( cut->GetPoints() );
    polygon->InsertNextCell( VTK_POLYGON, polyLine->GetPointIds() );

    vtkSmartPointer<vtkTriangleFilter> triangulate =
      vtkSmartPointer<vtkTriangleFilter>::New();
    triangulate->SetInputData( polygon );
    triangulate->Update();

    append->AddInputConnection( triangulate->GetOutputPort() );

    // Now measure the surface area of the cross section
    vtkSmartPointer<vtkMassProperties> massProperties =
      vtkSmartPointer<vtkMassProperties>::New();
    massProperties->SetInputConnection( triangulate->GetOutputPort() );
    massProperties->Update();

    std::cout << "surface area: " << massProperties->GetSurfaceArea()
              << std::endl;
    }

  vtkSmartPointer<vtkXMLPolyDataWriter> cutterWriter =
    vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  cutterWriter->SetFileName( "cut.vtp" );
  cutterWriter->SetInputConnection( append->GetOutputPort() );
  cutterWriter->Update();

  return EXIT_SUCCESS;
}
