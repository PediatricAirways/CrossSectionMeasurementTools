/*=============================================================================
//  --- Airway Segmenter ---+
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
//  Authors: Schuyler Kylstra
=============================================================================*/

#ifndef DicomToNrrd_ProgramArguments_h_included
#define DicomToNrrd_ProgramArguments_h_included

namespace DicomToNrrd {

/** Simple container class for holding program arguments.
 *
 * For information about what each member is, please see
 * DicomToNrrd.xml */
class ProgramArguments {
public:
  std::string dicomDir;

  std::string outputImage;
};

} // end namespace DicomToNrrd


#endif 