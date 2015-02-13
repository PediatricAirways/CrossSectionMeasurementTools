#ifndef itkAirwayLaplaceBoundaryImageFilter_hxx_included
#define itkAirwayLaplaceBoundaryImageFilter_hxx_included

#include "itkAirwayLaplaceBoundaryImageFilter.h"
#include "itkImageRegionIteratorWithIndex.h"
#include <itkInvertIntensityImageFilter.h>
#include <itkBinaryContourImageFilter.h>
#include "itkImageDuplicator.h"
#include "itkVector.h"
#include <cmath>

namespace itk
{

template< typename TInputImage>
AirwayLaplaceBoundaryImageFilter< TInputImage >
::AirwayLaplaceBoundaryImageFilter()
{
  m_InteriorAirwayBoundaryConditionLabel  = 11;
  m_NostrilBoundaryConditionLabel         = 4;
  m_TracheaBoundaryConditionLabel         = 5;
  m_NeumannBoundaryConditionLabel         = 6;
}


template< typename TInputImage>
void
AirwayLaplaceBoundaryImageFilter< TInputImage>
::GenerateData()
{

  typename InputImageType::ConstPointer  input  = this->GetInput();
  typename InputImageType::Pointer       output = this->GetOutput();

  this->AllocateOutputs();
  output->FillBuffer( 0 );

  // Copy the Input to the intermediate
  typedef itk::ImageDuplicator< InputImageType > DuplicatorType;
  typename DuplicatorType::Pointer duplicator = DuplicatorType::New();
  duplicator->SetInputImage( input ); //->GetPointer() );
  duplicator->Update();
  typename InputImageType::Pointer airway  = duplicator->GetOutput();

  // Cut off the nostrils and the trachea carina
  typedef itk::ImageRegionIteratorWithIndex< InputImageType > ImageIterator;

  // Iterate within a region near the nostrils to avoid cutting off
  // other parts of the airway.
  typename InputImageType::PointType nostrilLowPoint, nostrilHighPoint;
  typename InputImageType::PointType carinaLowPoint, carinaHighPoint;
  double regionRadius = 20.0;
  for ( unsigned int i = 0; i < InputImageType::ImageDimension; ++i )
    {
    nostrilLowPoint[i]  = m_NasalPoint[i] - regionRadius;
    nostrilHighPoint[i] = m_NasalPoint[i] + regionRadius;
    carinaLowPoint[i]   = m_TracheaPoint[i] - regionRadius;
    carinaHighPoint[i]  = m_TracheaPoint[i] + regionRadius;
    }

  typename InputImageType::IndexType nostrilLowIndex, nostrilHighIndex;
  airway->TransformPhysicalPointToIndex( nostrilLowPoint, nostrilLowIndex );
  airway->TransformPhysicalPointToIndex( nostrilHighPoint, nostrilHighIndex );

  typename InputImageType::IndexType carinaLowIndex, carinaHighIndex;
  airway->TransformPhysicalPointToIndex( carinaLowPoint, carinaLowIndex );
  airway->TransformPhysicalPointToIndex( carinaHighPoint, carinaHighIndex );

  typename InputImageType::RegionType nostrilRegion;
  nostrilRegion.SetIndex( nostrilLowIndex );
  nostrilRegion.SetUpperIndex( nostrilHighIndex );
  nostrilRegion.Crop( airway->GetLargestPossibleRegion() );

  typename InputImageType::RegionType carinaRegion;
  carinaRegion.SetIndex( carinaLowIndex );
  carinaRegion.SetUpperIndex( carinaHighIndex );
  carinaRegion.Crop( airway->GetLargestPossibleRegion() );

  // Compute d in equation ax + by + cz + d = 0
  double nostrilD = 0.0;
  double carinaD = 0.0;
  for ( unsigned int i = 0; i < InputImageType::ImageDimension; ++i )
    {
    nostrilD -= m_NasalVector[i]   * m_NasalPoint[i];
    carinaD  -= m_TracheaVector[i] * m_TracheaPoint[i];
    }

  ImageIterator nostrilIt( airway, nostrilRegion );
  for ( nostrilIt.GoToBegin(); !nostrilIt.IsAtEnd(); ++nostrilIt )
    {
    typename InputImageType::PointType inputPoint;
    airway->TransformIndexToPhysicalPoint( nostrilIt.GetIndex(), inputPoint );

    double nostrilDotProduct = 0.0;
    for ( unsigned int i = 0; i < InputImageType::ImageDimension; ++i )
      {
      nostrilDotProduct += m_NasalVector[i] * inputPoint[i];
      }

    // Apply plane equation test
    if ( nostrilDotProduct + nostrilD  < 0 )
      {
      nostrilIt.Set( 0 );
      }
    }

  ImageIterator carinaIt( airway, carinaRegion );
  for ( carinaIt.GoToBegin(); !carinaIt.IsAtEnd(); ++carinaIt )
    {
    typename InputImageType::PointType inputPoint;
    airway->TransformIndexToPhysicalPoint( carinaIt.GetIndex(), inputPoint );

    double carinaDotProduct = 0.0;
    for ( unsigned int i = 0; i < InputImageType::ImageDimension; ++i )
      {
      carinaDotProduct += m_TracheaVector[i] * inputPoint[i];
      }

    // Apply plane equation test
    if ( carinaDotProduct + carinaD  < 0 )
      {
      carinaIt.Set( 0 );
      }
    }

  // find the boundary
  typedef itk::BinaryContourImageFilter < InputImageType, InputImageType > ContourFilter;
  typename ContourFilter::Pointer contourFilter =  ContourFilter::New();
  contourFilter->SetInput( airway );
  contourFilter->SetForegroundValue( 0 );
  contourFilter->SetBackgroundValue( 1 );
  contourFilter->Update();

  // Invert intensity
  typedef itk::InvertIntensityImageFilter < InputImageType, InputImageType > InvertFilter;
  typename InvertFilter::Pointer invertFilter = InvertFilter::New();
  invertFilter->SetInput( contourFilter->GetOutput() );
  invertFilter->SetMaximum(1);
  invertFilter->Update();

  typename InputImageType::Pointer airwayBoundary = invertFilter->GetOutput();

  // Final classification
  ImageIterator airwayIt( airway,
                          airway->GetLargestPossibleRegion() );
  ImageIterator boundaryIt( airwayBoundary,
                            airwayBoundary->GetLargestPossibleRegion() );
  ImageIterator outputIt( output,
                          output->GetLargestPossibleRegion() );
  while ( !airwayIt.IsAtEnd() &&
          !boundaryIt.IsAtEnd() &&
          !outputIt.IsAtEnd() )
    {
    if ( airwayIt.Get() == 1 )
      {
      outputIt.Set( m_InteriorAirwayBoundaryConditionLabel );
      }
    else if ( boundaryIt.Get() == 1 )
      {
      typename InputImageType::PointType inputPoint;
      airway->TransformIndexToPhysicalPoint( airwayIt.GetIndex(),
                                             inputPoint );

      // Set as Neumann boundary condition by default
      outputIt.Set( m_NeumannBoundaryConditionLabel );

      // But check to see if voxel should be Dirichlet boundary condition
      if ( nostrilRegion.IsInside( airwayIt.GetIndex() ) )
        {
        // Do dot product test again. Voxels above the plane on the
        // boundary should be marked with the nostril boundary
        // condition.
        double nostrilDotProduct = 0.0;
        for ( unsigned int i = 0; i < InputImageType::ImageDimension; ++i )
          {
          nostrilDotProduct += m_NasalVector[i] * inputPoint[i];
          }

        if ( nostrilDotProduct + nostrilD  < 0 )
          {
          outputIt.Set( m_NostrilBoundaryConditionLabel );
          }
        }
      else if ( carinaRegion.IsInside( airwayIt.GetIndex() ) )
        {
        // Do dot product test again. Voxels above the plane on the
        // boundary should be marked with the carina boundary
        // condition.
        double carinaDotProduct = 0.0;
        for ( unsigned int i = 0; i < InputImageType::ImageDimension; ++i )
          {
          carinaDotProduct += m_TracheaVector[i] * inputPoint[i];
          }

        if ( carinaDotProduct + carinaD  < 0 )
          {
          outputIt.Set( m_TracheaBoundaryConditionLabel );
          }
        }
      }
    else
      {
      // Exterior
      outputIt.Set( 0 );
      }

    ++airwayIt;
    ++boundaryIt;
    ++outputIt;
    }
}

} // end namespace itk

#endif
