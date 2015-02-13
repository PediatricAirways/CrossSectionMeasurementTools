#ifndef itkAirwayLaplaceSolutionFilter_hxx_included
#define itkAirwayLaplaceSolutionFilter_hxx_included

#include "itkAirwayLaplaceSolutionFilter.h"
#include "itkAirwayLaplaceBoundaryImageFilter.h"
#include "itkLaplaceEquationSolverImageFilter.h"
#include "itkImageRegionIteratorWithIndex.h"
#include <itkInvertIntensityImageFilter.h>
#include <itkBinaryContourImageFilter.h>
#include "itkImageDuplicator.h"
#include "itkVector.h"
#include <stdio.h>
#include <math.h>

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

  // Identify Boundary
  typedef itk::AirwayLaplaceBoundaryImageFilter< TInputImage > BoundaryFilter;
  typename BoundaryFilter::Pointer boundaryFilter = BoundaryFilter::New();
  boundaryFilter->SetInput( input );
  boundaryFilter->SetNasalPoint(    m_NosePoint   );
  boundaryFilter->SetNasalVector(   m_NoseVector  );
  boundaryFilter->SetTracheaPoint(  m_TrachPoint  );
  boundaryFilter->SetTracheaVector( m_TrachVector );
  boundaryFilter->Update();


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
