/*=============================================================================
//  --- Compute Heat Contours ---+
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
//  Authors: Cory Quammen
=============================================================================*/

#include "ComputeHeatContoursCLP.h"

#include <vtkContourFilter.h>
#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLUnstructuredGridReader.h>

/*******************************************************************/
// Compute heat contours from the thresholded heat image
int main( int argc, char* argv[])
{
  PARSE_ARGS;

  vtkSmartPointer<vtkXMLUnstructuredGridReader> reader =
    vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
  reader->SetFileName(input.c_str());

  vtkIdType numContours = 1000;
  vtkSmartPointer<vtkContourFilter> contourFilter =
    vtkSmartPointer<vtkContourFilter>::New();
  contourFilter->GenerateValues( numContours, 0.0, 1.0 );
  contourFilter->SetInputConnection( reader->GetOutputPort() );

  vtkSmartPointer<vtkXMLPolyDataWriter> writer =
    vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  writer->SetInputConnection(contourFilter->GetOutputPort());
  writer->SetFileName(output.c_str());
  writer->Write();

  return EXIT_SUCCESS;
}

