/*=============================================================================
//  --- Split Epiglottis Cross Section ---+
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
//  Authors: Tim Thirion, Cory Quammen
=============================================================================*/

#include <vtkClipPolyData.h>
#include <vtkDataArray.h>
#include <vtkDelimitedTextReader.h>
#include <vtkFieldData.h>
#include <vtkGeometryFilter.h>
#include <vtkLongArray.h>
#include <vtkMassProperties.h>
#include <vtkPlane.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTable.h>
#include <vtkThreshold.h>
#include <vtkVariantArray.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>

#include <cassert>
#include <iostream>

#define VERBOSE 0

// Useful metadata
static double epiglottisTip[3];
static double noseTip[3];

namespace {

int processMetadata(const char *path)
{
  std::ostringstream oss;

  // Skip commented lines in the CSV
  std::ifstream in(path);
  std::string line;
  while (std::getline(in, line))
    {
    if (line[0] != '#')
      {
      oss << line << std::endl;
      }
    }

  // Process CSV of metadata
  vtkSmartPointer<vtkDelimitedTextReader> reader =
    vtkSmartPointer<vtkDelimitedTextReader>::New();
  reader->SetFieldDelimiterCharacters(",");
  reader->DetectNumericColumnsOn();
  reader->SetHaveHeaders(false);
  reader->ReadFromInputStringOn();
  reader->SetInputString(oss.str().c_str());
  reader->Update();

  vtkTable *table = reader->GetOutput();
  assert(table);

  bool noseTipFound = false;
  bool epiglottisTipFound = false;
  for (vtkIdType i = 0; i < table->GetNumberOfRows(); ++i)
    {
    vtkVariantArray *row = table->GetRow(i);
    if (!strcmp(row->GetValue(11).ToString(), "NoseTip"))
      {
      noseTip[0] = row->GetValue(1).ToDouble();
      noseTip[1] = row->GetValue(2).ToDouble();
      noseTip[2] = row->GetValue(3).ToDouble();
      noseTipFound = true;
      }
    if (!strcmp(row->GetValue(11).ToString(), "EpiglottisTip"))
      {
      epiglottisTip[0] = row->GetValue(1).ToDouble();
      epiglottisTip[1] = row->GetValue(2).ToDouble();
      epiglottisTip[2] = row->GetValue(3).ToDouble();
      epiglottisTipFound = true;
      }
    }

  if (!noseTipFound)
    {
    std::cerr << "Could not find landmark NoseTip\n";
    }

  if (!epiglottisTipFound)
    {
    std::cerr << "Could not find landmark EpiglottisTip\n";
    }

  return (noseTipFound && epiglottisTipFound) ? EXIT_SUCCESS : EXIT_FAILURE;
}

} // end anonymous namespace

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  if (argc != 6)
    {
    std::cerr
      << "Usage: " << argv[0]
      << "[vtp] [csv] [areas txt] [front vtp] [back vtp]"
      << std::endl;
    return -1;
    }

  int success = processMetadata(argv[2]);
  if (success == EXIT_FAILURE)
    {
    return success;
    }

  // Read VTP
  vtkSmartPointer<vtkXMLPolyDataReader> reader =
    vtkSmartPointer<vtkXMLPolyDataReader>::New();
  reader->SetFileName(argv[1]);
  reader->Update();
  vtkFieldData *fieldData = reader->GetOutput()->GetFieldData();

  vtkAbstractArray *abstractNames = fieldData->GetAbstractArray("query point name");
  assert(abstractNames);
  vtkStringArray *names = vtkStringArray::SafeDownCast(abstractNames);
  assert(names);
  vtkIdType index = names->LookupValue("EpiglottisTip");
  if (index < 0)
    {
    std::cerr << "No EpiglottisTip landmark found in '" << argv[2] << "'\n";
    return EXIT_FAILURE;
    }

  vtkLongArray *ids = vtkLongArray::SafeDownCast(
      fieldData->GetArray("contour ID"));
  assert(ids);
  long contourId = ids->GetTuple1(index);

  vtkDataArray *normals = fieldData->GetArray("normal");
  assert(normals);
  double *sectionNormal = normals->GetTuple3(index);

  // Compute the cut plane
  double projected[3] = { 0.0 };
  vtkPlane::GeneralizedProjectPoint(noseTip, epiglottisTip, sectionNormal,
      projected);
  double cutNormal[3] = {
    projected[0] - epiglottisTip[0],
    projected[1] - epiglottisTip[1],
    projected[2] - epiglottisTip[2]
  };
  vtkSmartPointer<vtkPlane> cutPlane =
    vtkSmartPointer<vtkPlane>::New();
  cutPlane->SetOrigin(epiglottisTip);
  cutPlane->SetNormal(cutNormal);

  // Threshold out all but the EpiglottisTip ID
  vtkSmartPointer<vtkThreshold> threshold =
    vtkSmartPointer<vtkThreshold>::New();
  //threshold->SetAttributeModeToUseCellData();
  threshold->SetInputConnection(reader->GetOutputPort());
  threshold->SetInputArrayToProcess(0, 0, 0,
      vtkDataObject::FIELD_ASSOCIATION_CELLS, "contour ID");
  threshold->ThresholdBetween(contourId, contourId);
  threshold->Update();

  // vtkUnstructuredGrid -> vtkPolyData
  vtkSmartPointer<vtkGeometryFilter> convert =
    vtkSmartPointer<vtkGeometryFilter>::New();
  convert->SetInputConnection(threshold->GetOutputPort());
  convert->Update();

  // Cut the thresholded output by the computed plane
  vtkSmartPointer<vtkClipPolyData> clip =
    vtkSmartPointer<vtkClipPolyData>::New();
  clip->SetInputConnection(convert->GetOutputPort());
  clip->GenerateClippedOutputOn();
  clip->SetClipFunction(cutPlane);
  clip->Update();

  // Compute areas of clipped regions
  vtkSmartPointer<vtkMassProperties> frontMass =
    vtkSmartPointer<vtkMassProperties>::New();
  frontMass->SetInputConnection(clip->GetOutputPort());
  double frontArea = frontMass->GetSurfaceArea();

  vtkSmartPointer<vtkMassProperties> backMass =
    vtkSmartPointer<vtkMassProperties>::New();
  backMass->SetInputConnection(clip->GetClippedOutputPort());
  double backArea = backMass->GetSurfaceArea();

  // Write text file of two areas
  std::ofstream results(argv[3]);
  results << frontArea << std::endl;
  results << backArea << std::endl;

  // Write front and back VTPs
  vtkSmartPointer<vtkXMLPolyDataWriter> frontWriter =
    vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  frontWriter->SetInputConnection(clip->GetOutputPort());
  frontWriter->SetFileName(argv[4]);
  frontWriter->Update();

  vtkSmartPointer<vtkXMLPolyDataWriter> backWriter =
    vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  backWriter->SetInputConnection(clip->GetClippedOutputPort());
  backWriter->SetFileName(argv[5]);
  backWriter->Update();

  // Bunch of debug output
#if VERBOSE
  std::cout << "Contour ID: " << contourId << std::endl;
  std::cout << "EpiglottisTip: " << epiglottisTip[0] << ", "
            << epiglottisTip[1] << ", "
            << epiglottisTip[2] << std::endl;
  std::cout << "Cross section normal: " << sectionNormal[0] << ", "
            << sectionNormal[1] << ", "
            << sectionNormal[2] << std::endl;
  std::cout << "NoseTip Projected: " << projected[0] << ", "
            << projected[1] << ", "
            << projected[2] << std::endl;
  std::cout << "Front area: " << frontArea << std::endl;
  std::cout << "Back area: " << backArea << std::endl;
  std::cout << "Ratio: " << (frontArea / backArea) << std::endl;
#endif

  return 0;
}
