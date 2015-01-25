#include <iostream>

#include "vtkContourAtPointsFilter.h"

#include <vtkCellArray.h>
#include <vtkIdList.h>
#include <vtkImageThreshold.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkSmartPointer.h>
#include <vtkThreshold.h>
#include <vtkTriangle.h>
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
  polyDataWriter->SetFileName( argv[2] );
  polyDataWriter->SetInputConnection( connected->GetOutputPort() );
  polyDataWriter->Update();

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
    }

  return EXIT_SUCCESS;
}
