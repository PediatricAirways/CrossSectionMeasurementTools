#include <iostream>

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImageRegionIterator.h>

#include "itkLaplaceEquationSolverImageFilter.h"

int main(int argc, char* argv[])
{
  typedef itk::Image< unsigned int, 3 > InputImageType;
  typedef itk::Image< float, 3 >        OutputImageType;

  typedef itk::LaplaceEquationSolverImageFilter< InputImageType,
                                    OutputImageType > LaplaceEquationSolverFilterType;

#if 0
  // Set up a dummy image for testing
  InputImageType::SizeType size = {{10, 10, 10}};
  InputImageType::RegionType region( size );
  InputImageType::Pointer input = InputImageType::New();
  input->SetRegions( region );
  input->Allocate();
  input->FillBuffer( 0 );

  // Set up square low temperature region
  InputImageType::IndexType  lowTempIndex = {{ 2, 2, 1 }};
  InputImageType::SizeType   lowTempSize = {{ 6, 6, 1 }};
  InputImageType::RegionType lowTempRegion( lowTempIndex, lowTempSize );

  typedef itk::ImageRegionIterator< InputImageType > InputIteratorType;
  InputIteratorType iter( input, lowTempRegion );
  for ( iter.GoToBegin(); !iter.IsAtEnd(); ++iter )
    {
    iter.Set( 4 );
    }

  // Set up square high temperature region
  InputImageType::IndexType  highTempIndex = {{ 2, 2, 8 }};
  InputImageType::SizeType   highTempSize = {{ 6, 6, 1 }};
  InputImageType::RegionType highTempRegion( highTempIndex, highTempSize );

  InputIteratorType iterHigh( input, highTempRegion );
  for ( iterHigh.GoToBegin(); !iterHigh.IsAtEnd(); ++iterHigh )
    {
    iterHigh.Set( 5 );
    }

  // Set up Neumann boundary condition
  InputImageType::IndexType  neumannIndex = {{ 1, 2, 2 }};
  InputImageType::SizeType   neumannSize = {{ 1, 6, 6 }};
  InputImageType::RegionType neumannRegion( neumannIndex, neumannSize );

  InputIteratorType iterNeumannLeft( input, neumannRegion );
  for ( iterNeumannLeft.GoToBegin(); !iterNeumannLeft.IsAtEnd(); ++iterNeumannLeft )
    {
    iterNeumannLeft.Set( 6 );
    }

  neumannIndex[0] = 8;
  neumannRegion.SetIndex( neumannIndex );

  InputIteratorType iterNeumannRight( input, neumannRegion );
  for ( iterNeumannRight.GoToBegin(); !iterNeumannRight.IsAtEnd(); ++iterNeumannRight )
    {
    iterNeumannRight.Set( 6 );
    }

  neumannIndex[0] = 2;
  neumannIndex[1] = 1;
  neumannIndex[2] = 2;
  neumannSize[0] = 6;
  neumannSize[1] = 1;
  neumannSize[2] = 6;
  neumannRegion.SetIndex( neumannIndex );
  neumannRegion.SetSize( neumannSize );

  InputIteratorType iterNeumannFront( input, neumannRegion );
  for ( iterNeumannFront.GoToBegin(); !iterNeumannFront.IsAtEnd(); ++iterNeumannFront )
    {
    iterNeumannFront.Set( 6 );
    }

  neumannIndex[1] = 8;
  neumannRegion.SetIndex( neumannIndex );

  InputIteratorType iterNeumannBack( input, neumannRegion );
  for ( iterNeumannBack.GoToBegin(); !iterNeumannBack.IsAtEnd(); ++iterNeumannBack )
    {
    iterNeumannBack.Set( 6 );
    }

  // Now set up solution region
  InputImageType::IndexType  solutionIndex = {{ 2, 2, 2 }};
  InputImageType::SizeType   solutionSize = {{6, 6, 6 }};
  InputImageType::RegionType solutionRegion( solutionIndex, solutionSize );

  InputIteratorType iterSolution( input, solutionRegion );
  for ( iterSolution.GoToBegin(); !iterSolution.IsAtEnd(); ++iterSolution )
    {
    iterSolution.Set( 11 );
    }

#endif

  typedef itk::ImageFileReader< InputImageType > InputImageReaderType;
  InputImageReaderType::Pointer reader = InputImageReaderType::New();
  reader->SetFileName( "airway.nrrd" );
  try
    {
    reader->Update();
    }
  catch ( itk::ExceptionObject & exception )
    {
    std::cerr << exception << std::endl;
    return EXIT_FAILURE;
    }

  LaplaceEquationSolverFilterType::Pointer heatFlowFilter
    = LaplaceEquationSolverFilterType::New();
  //heatFlowFilter->SetInput( input );
  heatFlowFilter->SetSolutionLabel( 11 );
  heatFlowFilter->SetNeumannBoundaryConditionLabel( 6 );
  heatFlowFilter->AddDirichletBoundaryCondition( 4, 0.0 );
  heatFlowFilter->AddDirichletBoundaryCondition( 5, 1.0 );
  heatFlowFilter->SetInput( reader->GetOutput() );
  try
    {
    heatFlowFilter->UpdateLargestPossibleRegion();
    }
  catch ( itk::ExceptionObject & exception )
    {
    std::cerr << exception << std::endl;
    }

#if 0
  // Write out input image
  typedef itk::ImageFileWriter< InputImageType > InputWriterType;
  InputWriterType::Pointer inputWriter = InputWriterType::New();
  inputWriter->SetFileName( "input.nrrd" );
  inputWriter->SetInput( input );
  try
    {
    inputWriter->Update();
    }
  catch ( itk::ExceptionObject & exception )
    {
    std::cerr << "Error writing to 'input.nrrd'." << std::endl;
    std::cerr << exception << std::endl;
    }
#endif

  // Write out output image
  typedef itk::ImageFileWriter< OutputImageType > OutputWriterType;
  OutputWriterType::Pointer outputWriter = OutputWriterType::New();
  outputWriter->SetFileName( "output.nrrd" );
  outputWriter->SetInput( heatFlowFilter->GetOutput() );
  try
    {
    outputWriter->Update();
    }
  catch ( itk::ExceptionObject & exception )
    {
    std::cerr << "Error writing to 'output.nrrd'." << std::endl;
    std::cerr << exception << std::endl;
    }

  return EXIT_SUCCESS;
}
