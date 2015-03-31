#ifndef itkAirwayLaplaceSolutionFilter_hxx_included
#define itkAirwayLaplaceSolutionFilter_hxx_included

#include "itkAirwayLaplaceSolutionFilter.h"
#include "itkAirwayLaplaceBoundaryImageFilter.h"
#include <itkBinaryThresholdImageFilter.h>
#include <itkConnectedComponentImageFilter.h>
#include "itkLaplaceEquationSolverImageFilter.h"
#include <itkRelabelComponentImageFilter.h>
#include "itkImageRegionIteratorWithIndex.h"
#include <itkInvertIntensityImageFilter.h>
#include <itkBinaryContourImageFilter.h>
#include "itkImageDuplicator.h"
#include "itkVector.h"
#include <stdio.h>
#include <math.h>

// TEMPORARY
#include <itkImageFileWriter.h>

namespace itk
{

template< typename TInputImage, typename TOutputImage >
AirwayLaplaceSolutionFilter< TInputImage, TOutputImage >
::AirwayLaplaceSolutionFilter()
{

}


template< typename TInputImage, typename TOutputImage >
void
AirwayLaplaceSolutionFilter< TInputImage, TOutputImage >
::GenerateData()
{


  typename TInputImage::ConstPointer  input  = this->GetInput();
  typename TOutputImage::Pointer      output = this->GetOutput();

  this->AllocateOutputs();
  output->FillBuffer( 0 );

  // Extract largest connected component from binary image. If we don't
  // do this, the Laplace solution will be undefined (maybe).
  typedef itk::ConnectedComponentImageFilter< TInputImage, TInputImage, TInputImage > ConnectedFilterType;
  typename ConnectedFilterType::Pointer connectedFilter = ConnectedFilterType::New();
  connectedFilter->SetInput( input );

  typedef itk::RelabelComponentImageFilter< TInputImage, TInputImage > RelabelFilterType;
  typename RelabelFilterType::Pointer relabelFilter = RelabelFilterType::New();
  relabelFilter->SetInput( connectedFilter->GetOutput() );

  typedef itk::BinaryThresholdImageFilter< TInputImage, TInputImage > ThresholdType;
  typename ThresholdType::Pointer thresholdFilter = ThresholdType::New();
  thresholdFilter->SetInsideValue( 1 );
  thresholdFilter->SetOutsideValue( 0 );
  thresholdFilter->SetLowerThreshold( 1 );
  thresholdFilter->SetUpperThreshold( 1 );
  thresholdFilter->SetInput( relabelFilter->GetOutput() );

  // Identify Boundary
  typedef itk::AirwayLaplaceBoundaryImageFilter< TInputImage > BoundaryFilter;
  typename BoundaryFilter::Pointer boundaryFilter = BoundaryFilter::New();
  boundaryFilter->SetInput( thresholdFilter->GetOutput() );
  boundaryFilter->SetNasalPoint(    m_NosePoint   );
  boundaryFilter->SetNasalVector(   m_NoseVector  );
  boundaryFilter->SetTracheaPoint(  m_TrachPoint  );
  boundaryFilter->SetTracheaVector( m_TrachVector );
  boundaryFilter->Update();

  // TEMPORARY - write out boundary image
  typedef itk::ImageFileWriter< TInputImage > WriterType;
  typename WriterType::Pointer writer = WriterType::New();
  writer->SetFileName("BOUNDARY_CONDITION_IMAGE.mha");
  writer->SetInput( boundaryFilter->GetOutput() );
  writer->Update();

  typename WriterType::Pointer ccWriter = WriterType::New();
  ccWriter->SetFileName("CONNECTED_COMPONENTS_IMAGE.mha");
  ccWriter->SetInput( thresholdFilter->GetOutput() );
  ccWriter->Update();

  // // Solve Laplace Equation
  typedef itk::LaplaceEquationSolverImageFilter< TInputImage, TOutputImage > LaplaceFilterType;
  typename LaplaceFilterType::Pointer heatFlowFilter = LaplaceFilterType::New();
  heatFlowFilter->SetSolutionLabel( 11 );
  heatFlowFilter->SetNeumannBoundaryConditionLabel( 6      );
  heatFlowFilter->AddDirichletBoundaryCondition(    4, 0.0 );
  heatFlowFilter->AddDirichletBoundaryCondition(    5, 1.0 );
  heatFlowFilter->SetInput( boundaryFilter->GetOutput()    );
  heatFlowFilter->Update();


  // Export Result
  typedef itk::ImageRegionIteratorWithIndex< TOutputImage > ImageIterator;
  ImageIterator it( heatFlowFilter->GetOutput(), ( heatFlowFilter->GetOutput() )->GetRequestedRegion() );
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    const OutputIndexType index = it.GetIndex();
    output->SetPixel( index, ( heatFlowFilter->GetOutput() )->GetPixel( index ) );
  }

}

} // end namespace itk

#endif
