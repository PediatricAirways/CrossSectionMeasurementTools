/*=============================================================================
//  --- DICOM to NRRD coverter ---+
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

/* STL includes */
#include <string>

/* Local includes */
#include "ConvertDICOMToNRRDCLP.h"
#include "ConvertDICOMToNRRDConfig.h"
#include "ConvertDICOMToNRRD.hxx"
#include "ProgramArguments.h"

#include <itkImage.h>
#include <itkImageFileReader.h>

namespace DICOMToNRRD {

  /*******************************************************************/
  /** Query the image type. */
  /*******************************************************************/
  void GetImageType ( std::string dicomDirectory,
                      itk::ImageIOBase::IOComponentType & componentType)
  {
    typedef itk::Image<unsigned char, 3> ImageType;
    typedef itk::GDCMSeriesFileNames NamesGeneratorType;
    typedef std::vector< std::string >   SeriesIdContainer;
    typedef std::vector< std::string >   FileNamesContainer;
    itk::ImageFileReader<ImageType>::Pointer imageReader =
      itk::ImageFileReader<ImageType>::New();

    try {
      NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();
      nameGenerator->SetDirectory( dicomDirectory );

      const SeriesIdContainer & seriesUID = nameGenerator->GetSeriesUIDs();
      SeriesIdContainer::const_iterator seriesItr = seriesUID.begin();

      std::string seriesIdentifier;
      seriesIdentifier = seriesUID.begin()->c_str();

      FileNamesContainer fileNames;
      fileNames = nameGenerator->GetFileNames( seriesIdentifier );

      imageReader->SetFileName( fileNames.begin()->c_str() );
      imageReader->UpdateOutputInformation();

      componentType = imageReader->GetImageIO()->GetComponentType();
    } catch( itk::ExceptionObject & excep ) {
      std::cerr << "Exception caught !" << std::endl;
      std::cerr << excep << std::endl;
    }
  }
} // end namespace DICOMToNRRD

/*******************************************************************/
int main( int argc, char * argv[] )
{

  PARSE_ARGS;

  DICOMToNRRD::ProgramArguments args;
  args.dicomDir    = dicomDir;
  args.outputImage = outputImage;

  itk::ImageIOBase::IOPixelType     inputPixelType;
  itk::ImageIOBase::IOComponentType inputComponentType;

  int ret = EXIT_FAILURE;

  try {
    DICOMToNRRD::GetImageType(dicomDir, inputComponentType);
    (void) inputPixelType;

    switch( inputComponentType ) {
#if defined(SUPPORT_UCHAR_PIXEL)
      case itk::ImageIOBase::UCHAR:
        ret = DICOMToNRRD::ExecuteFromFile( args, static_cast<unsigned char>(0) );
        break;
#endif
#if defined(SUPPORT_CHAR_PIXEL)
      case itk::ImageIOBase::CHAR:
        ret = DICOMToNRRD::ExecuteFromFile( args, static_cast<char>(0) );
        break;
#endif
#if defined(SUPPORT_USHORT_PIXEL)
      case itk::ImageIOBase::USHORT:
        ret = DICOMToNRRD::ExecuteFromFile( args, static_cast<unsigned short>(0) );
        break;
#endif
#if defined(SUPPORT_SHORT_PIXEL)
      case itk::ImageIOBase::SHORT:
        ret = DICOMToNRRD::ExecuteFromFile( args, static_cast<short>(0) );
        break;
#endif
#if defined(SUPPORT_UINT_PIXEL)
      case itk::ImageIOBase::UINT:
        ret = DICOMToNRRD::ExecuteFromFile( args, static_cast<unsigned int>(0) );
        break;
#endif
#if defined(SUPPORT_INT_PIXEL)
      case itk::ImageIOBase::INT:
        ret = DICOMToNRRD::ExecuteFromFile( args, static_cast<int>(0) );
        break;
#endif
#if defined(SUPPORT_ULONG_PIXEL)
      case itk::ImageIOBase::ULONG:
        ret = DICOMToNRRD::ExecuteFromFile( args, static_cast<unsigned long>(0) );
        break;
#endif
#if defined(SUPPORT_LONG_PIXEL)
      case itk::ImageIOBase::LONG:
        ret = DICOMToNRRD::ExecuteFromFile( args, static_cast<long>(0) );
        break;
#endif
#if defined(SUPPORT_FLOAT_PIXEL)
      case itk::ImageIOBase::FLOAT:
        ret = DICOMToNRRD::ExecuteFromFile( args, static_cast<float>(0) );
        break;
#endif
#if defined(SUPPORT_DOUBLE_PIXEL)
      case itk::ImageIOBase::DOUBLE:
        ret = DICOMToNRRD::ExecuteFromFile( args, static_cast<double>(0) );
        break;
#endif
      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
      default:
        std::cout << "unknown component type" << std::endl;
        break;
    }
  } catch( itk::ExceptionObject & excep ) {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;

    return EXIT_FAILURE;
  }

  return ret;
}
