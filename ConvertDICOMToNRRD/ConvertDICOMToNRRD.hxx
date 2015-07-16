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
//  Authors: Schuyler Kylstra, Cory Quammen
=============================================================================*/
#ifndef DICOMToNRRD_hxx_included
#define DICOMToNRRD_hxx_included

#include <algorithm>
#include <cfloat>

/* ITK includes */
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>
#include <itkIdentityTransform.h>
#include <itkImage.h>
#include <itkImageFileWriter.h>
#include <itkImageSeriesReader.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkResampleImageFilter.h>
#include <itkSmartPointer.h>
#include <itkSpatialOrientationAdapter.h>

#include <itksys/Glob.hxx>

#include "ProgramArguments.h"


namespace DICOMToNRRD {

  /*******************************************************************/
  /** Run the algorithm on an input image and write it to the output */
  /** image.                                                         */
  /*******************************************************************/
  template< class TInput >
  int Execute( TInput * originalImage,
               itk::SmartPointer< TInput > & resampledInput,
               DICOMToNRRD::ProgramArguments args)
  {
    /* Typedefs */
    typedef typename TInput::PixelType TPixelType;
    const unsigned char DIMENSION = 3;

    typedef itk::Image<TPixelType, DIMENSION> InputImageType;

    /*  Automatic Resampling to RAI */
    typename InputImageType::DirectionType originalImageDirection = originalImage->GetDirection();

    itk::SpatialOrientationAdapter adapter;
    typename InputImageType::DirectionType RAIDirection = adapter.ToDirectionCosines(itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI);

    bool shouldConvert = false;

    for ( int i = 0; i < 3; ++i )
      {
      for ( int j = 0; j < 3; ++j )
        {
        if (std::abs(originalImageDirection[i][j] - RAIDirection[i][j]) > 1e-6)
          {
          shouldConvert = true;
          break;
          }
        }
      }

    typedef itk::ResampleImageFilter< InputImageType, InputImageType > ResampleImageFilterType;
    typename ResampleImageFilterType::Pointer resampleFilter = ResampleImageFilterType::New();

    if ( shouldConvert ) {
      typedef itk::IdentityTransform< double, DIMENSION > IdentityTransformType;

      // Figure out bounding box of rotated image
      double boundingBox[6] = { DBL_MAX, -DBL_MAX, DBL_MAX, -DBL_MAX, DBL_MAX, -DBL_MAX };
      typedef typename InputImageType::IndexType  IndexType;
      typedef typename InputImageType::RegionType RegionType;
      typedef typename InputImageType::PointType  PointType;

      RegionType region = originalImage->GetLargestPossibleRegion();
      IndexType lowerUpper[2];
      lowerUpper[0] = region.GetIndex();
      lowerUpper[1] = region.GetUpperIndex();

      for ( unsigned int i = 0; i < 8; ++i ) {
        IndexType cornerIndex;
        cornerIndex[0] = lowerUpper[ (i & 1u) >> 0 ][0];
        cornerIndex[1] = lowerUpper[ (i & 2u) >> 1 ][1];
        cornerIndex[2] = lowerUpper[ (i & 4u) >> 2 ][2];

        PointType point;
        originalImage->TransformIndexToPhysicalPoint( cornerIndex, point );
        boundingBox[0] = std::min( point[0], boundingBox[0] );
        boundingBox[1] = std::max( point[0], boundingBox[1] );
        boundingBox[2] = std::min( point[1], boundingBox[2] );
        boundingBox[3] = std::max( point[1], boundingBox[3] );
        boundingBox[4] = std::min( point[2], boundingBox[4] );
        boundingBox[5] = std::max( point[2], boundingBox[5] );
      }

      // Now transform the bounding box from physical space to index space
      PointType lowerPoint;
      lowerPoint[0] = boundingBox[0];
      lowerPoint[1] = boundingBox[2];
      lowerPoint[2] = boundingBox[4];

      PointType upperPoint;
      upperPoint[0] = boundingBox[1];
      upperPoint[1] = boundingBox[3];
      upperPoint[2] = boundingBox[5];

      typename InputImageType::Pointer dummyImage = InputImageType::New();
      dummyImage->SetOrigin( lowerPoint );
      dummyImage->SetSpacing( originalImage->GetSpacing() );
      dummyImage->SetLargestPossibleRegion( RegionType() );

      IndexType newLower, newUpper;
      dummyImage->TransformPhysicalPointToIndex( lowerPoint, newLower );
      dummyImage->TransformPhysicalPointToIndex( upperPoint, newUpper );

      RegionType outputRegion;
      outputRegion.SetIndex( newLower );
      outputRegion.SetUpperIndex( newUpper );

      // Find the minimum pixel value in the image. This will be used as the default value
      // in the resample filter.
      typedef itk::MinimumMaximumImageCalculator< InputImageType > MinMaxType;
      typename MinMaxType::Pointer minMaxCalculator = MinMaxType::New();
      minMaxCalculator->SetImage( originalImage );
      minMaxCalculator->Compute();

      resampleFilter->SetTransform( IdentityTransformType::New() );
      resampleFilter->SetInput( originalImage );
      resampleFilter->SetSize( outputRegion.GetSize() );
      resampleFilter->SetOutputOrigin( lowerPoint );
      resampleFilter->SetOutputSpacing( originalImage->GetSpacing() );
      resampleFilter->SetDefaultPixelValue( minMaxCalculator->GetMinimum() );
      resampleFilter->Update();

      originalImage = resampleFilter->GetOutput();
    }

    resampledInput = originalImage;

    return EXIT_SUCCESS;

  }

  std::string FindDICOMTag( const std::string & entryId, const itk::GDCMImageIO * dicomIO )
  {
    typedef itk::MetaDataDictionary DictionaryType;
    const  DictionaryType & dictionary = dicomIO->GetMetaDataDictionary();
    DictionaryType::ConstIterator tagItr = dictionary.Find( entryId );

    if ( tagItr == dictionary.End() )
      {
      return "NOT FOUND";
      }

    typedef itk::MetaDataObject< std::string > MetaDataStringType;

    MetaDataStringType::ConstPointer entryvalue = dynamic_cast<const MetaDataStringType *>( tagItr->second.GetPointer() );

    if ( entryvalue )
      {
      std::string tagvalue = entryvalue->GetMetaDataObjectValue();
      return tagvalue;
      }
    else
      {
        return "NOT FOUND";
      }
  }

  /***************************************************************/
  /* Execute the algorithm on an image read from a file.         */
  /***************************************************************/
  template <class T>
  int ExecuteFromFile( const ProgramArguments & args, T)
  {
    /* Typedefs */
    typedef T     TPixelType;

    const unsigned char DIMENSION = 3;
    typedef itk::Image<TPixelType, DIMENSION> InputImageType;
    typedef itk::GDCMImageIO                  ImageIOType;

    // Find a single DICOM file from which to get series information
    itksys::Glob glob;
    std::string globExpr = args.dicomDir + "/*.dcm";
    glob.FindFiles(globExpr);
    std::vector<std::string> & globFiles = glob.GetFiles();

    // File names from glob operation are unsorted, so we sort here.
    std::sort(globFiles.begin(), globFiles.end());
    std::string firstFile;
    if (globFiles.size() > 0)
      {
      firstFile = globFiles[0];
      }

    ImageIOType::Pointer dicomIO = ImageIOType::New();
    if (!dicomIO->CanReadFile( firstFile.c_str() ))
      {
      std::cerr << "Could not read file '" << firstFile << "'\n";
      return EXIT_FAILURE;
      }

    typedef itk::ImageSeriesReader< InputImageType > ReaderType;
    typedef std::vector< std::string >               SeriesIdContainer;
    typedef itk::ImageFileWriter< InputImageType >   WriterType;

    typename ReaderType::Pointer reader = ReaderType::New();
    reader->SetImageIO( dicomIO );
    
    typedef itk::GDCMSeriesFileNames NamesGeneratorType;
    NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();
    nameGenerator->SetUseSeriesDetails(false);
    nameGenerator->SetDirectory( args.dicomDir );
    try
      {
      typename ReaderType::Pointer singleReader = ReaderType::New();
      singleReader->SetFileName( firstFile.c_str() );

      typename ImageIOType::Pointer singleGDCMImageIO = ImageIOType::New();
      singleReader->SetImageIO( singleGDCMImageIO );
      singleReader->Update();
      std::string seriesInstance = FindDICOMTag( "0020|000e", singleGDCMImageIO );

      typedef std::vector< std::string > FileNamesContainer;
      FileNamesContainer fileNames( nameGenerator->GetFileNames( seriesInstance ) );
      reader->SetFileNames( fileNames );

      // Read the input file
      reader->Update();
      }
    catch (itk::ExceptionObject &ex)
      {
      std::cerr << "Exception caught when reading DICOM files: " << ex << std::endl;
      return EXIT_FAILURE;
      }
    

    // Run the algorithm
    typename InputImageType::Pointer resampledInput;
    int result = Execute( reader->GetOutput(), resampledInput, args );
    if ( result != EXIT_SUCCESS ) {
      return result;
    }

    // Write the result.
    typename WriterType::Pointer writer = WriterType::New();
    writer->SetInput( resampledInput );
    writer->SetFileName( args.outputImage );
    writer->Update();

    return EXIT_SUCCESS;
  }

}

#endif
