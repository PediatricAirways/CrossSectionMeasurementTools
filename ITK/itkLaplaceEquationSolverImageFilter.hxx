#ifndef itkLaplaceEquationSolverImageFilter_hxx_included
#define itkLaplaceEquationSolverImageFilter_hxx_included

#include "itkLaplaceEquationSolverImageFilter.h"

#include "itkImageRegionConstIteratorWithIndex.h"

#include <algorithm>
#include <ctime>
#include <limits>

#include <Eigen/SparseCholesky>

namespace itk
{

template< typename TInputImage, typename TOutputImage >
inline void
LaplaceEquationSolverImageFilter< TInputImage, TOutputImage >
::UpdateAB( size_t i, InputIndexType voxelIndex, OutputPixelType invDxDx,
            TripletListType & tripletList, VectorType & b,
            OffsetValueType solutionIndex )
{
  bool labelFound = false;
  InputPixelType label = this->GetInput()->GetPixel( voxelIndex );
  if ( label == m_SolutionLabel )
    {
    Eigen::Triplet< OutputPixelType > triplet( i, solutionIndex, invDxDx );
    tripletList.push_back( triplet );
    labelFound = true;
    }
  else if ( label == m_NeumannBoundaryConditionLabel )
    {
    Eigen::Triplet< OutputPixelType > triplet( i, i, invDxDx );
    tripletList.push_back( triplet );
    labelFound = true;
    }
  else
    {
    typename DirichletMapType::iterator iter =
      m_DirichletBoundaryConditionMap.find( label );
    if ( iter != m_DirichletBoundaryConditionMap.end() )
      {
      b[i] -= iter->second * invDxDx;
      labelFound = true;
      }
    }
  if ( !labelFound )
    {
    itkExceptionMacro( << "Unknown label value "
                       << label << " at index " << voxelIndex );
    }
}

template< typename TInputImage, typename TOutputImage >
LaplaceEquationSolverImageFilter< TInputImage, TOutputImage >
::LaplaceEquationSolverImageFilter()
{
  m_SolutionLabel = 11;
  m_NeumannBoundaryConditionLabel = 6;
}

template< typename TInputImage, typename TOutputImage >
void
LaplaceEquationSolverImageFilter< TInputImage, TOutputImage >
::ResetDirichletBoundaryConditions()
{
  m_DirichletBoundaryConditionMap.reset();

  this->Modified();
}

template< typename TInputImage, typename TOutputImage >
void
LaplaceEquationSolverImageFilter< TInputImage, TOutputImage >
::AddDirichletBoundaryCondition( InputPixelType boundaryConditionLabel,
                                 OutputPixelType boundaryConditionValue )
{
  m_DirichletBoundaryConditionMap[ boundaryConditionLabel ] =
    boundaryConditionValue;

  this->Modified();
}

template< typename TInputImage, typename TOutputImage >
size_t
LaplaceEquationSolverImageFilter< TInputImage, TOutputImage >
::GetNumberOfDirichletBoundaryConditions() const
{
  return m_DirichletBoundaryConditionMap.size();
}

template< typename TInputImage, typename TOutputImage >
void
LaplaceEquationSolverImageFilter< TInputImage, TOutputImage >
::GenerateData()
{
  typename TInputImage::ConstPointer input = this->GetInput();
  typename TOutputImage::Pointer output = this->GetOutput();

  // Allocate the output and initialize to NaN
  this->AllocateOutputs();
  output->FillBuffer( std::numeric_limits< OutputPixelType >::quiet_NaN() );

  // Do Laplace solution by iterative solver using 6 neighborhood
  InputIndexType minIndex = input->GetBufferedRegion().GetIndex();
  InputIndexType maxIndex = input->GetBufferedRegion().GetUpperIndex();

  // First get all the solution domain indices. We do this, otherwise
  // the matrices to solve will be far too large to fit into memory.
  std::vector< InputIndexType > solutionIndices;

  typedef typename TInputImage::OffsetValueType OffsetValueType;
  std::vector< OffsetValueType > linearIndex2UnknownIndex(
    input->GetPixelContainer()->Size(), 0 );

  long counter = 0;
  typedef ImageRegionConstIteratorWithIndex< TInputImage > IteratorWithIndexType;
  IteratorWithIndexType fullRegionIter( input.GetPointer(), input->GetBufferedRegion() );
  for ( fullRegionIter.GoToBegin(); !fullRegionIter.IsAtEnd(); ++fullRegionIter )
    {
    InputPixelType pixelValue = fullRegionIter.Get();
    InputIndexType index = fullRegionIter.GetIndex();
    if ( pixelValue == m_SolutionLabel )
      {
      solutionIndices.push_back( index );
      linearIndex2UnknownIndex[ input->ComputeOffset( fullRegionIter.GetIndex() ) ] = counter;
      ++counter;
      }
    }

  itkDebugMacro( << "Done with noting indices" );

  size_t numberOfUnknowns = solutionIndices.size();

  itkDebugMacro( << "Number of unknowns: " << numberOfUnknowns );

  // Set up the linear system by going through all the indices of the
  // solution domain and creating the sparse matrix A and the right
  // vector b
  typedef Eigen::SparseMatrix< OutputPixelType, Eigen::ColMajor > MatrixType;
  MatrixType A( numberOfUnknowns, numberOfUnknowns );

  // Set up the b vector
  typedef Eigen::Matrix< OutputPixelType, Eigen::Dynamic, 1 > VectorType;
  VectorType b( numberOfUnknowns );
  b.fill( 0.0 );

  // Set the diagonal elements
  OutputPixelType dx = input->GetSpacing()[0];
  OutputPixelType dy = input->GetSpacing()[1];
  OutputPixelType dz = input->GetSpacing()[2];

  OutputPixelType invDxDx = 1.0 / (dx * dx);
  OutputPixelType invDyDy = 1.0 / (dy * dy);
  OutputPixelType invDzDz = 1.0 / (dz * dz);

  itkDebugMacro( << "Done setting diagonal elements of A" );

  // Triplets to feed into A via
  // Eigen::SparseMatrix<>::setFromTriplets.  Note that triplets with
  // the same i, j indices will be summed in the sparse matrix.
  typedef Eigen::Triplet< OutputPixelType > Triplet;
  std::vector< Triplet > tripletList;
  tripletList.reserve( 7 * numberOfUnknowns );

  // This is somewhat of a goofy way to set up A. It is nearly a
  // direct translation from MATLAB code. Setting the elements based
  // on a traditional stencil approach might be faster and use less
  // memory.
  for ( size_t i = 0; i < numberOfUnknowns; ++i )
    {
    tripletList.push_back( Triplet( i, i, -2.0 * ( invDxDx + invDyDy + invDzDz ) ) );
    }

  for ( size_t i = 0; i < numberOfUnknowns; ++i )
    {
    InputIndexType index = solutionIndices[i];

    // +x, -x
    InputIndexType indexXP = index;
    indexXP[0] = std::min( maxIndex[0], index[0] + 1 );
    InputIndexType indexXM = index;
    indexXM[0] = std::max( minIndex[0], index[0] - 1 );

    // +y, -y
    InputIndexType indexYP = index;
    indexYP[1] = std::min( maxIndex[1], index[1] + 1 );
    InputIndexType indexYM = index;
    indexYM[1] = std::max( minIndex[1], index[1] - 1 );

    // +z, -z
    InputIndexType indexZP = index;
    indexZP[2] = std::min( maxIndex[2], index[2] + 1 );
    InputIndexType indexZM = index;
    indexZM[2] = std::max( minIndex[2], index[2] - 1 );

    OffsetValueType linearIndex;
    linearIndex = input->ComputeOffset( indexXP );
    this->UpdateAB( i, indexXP, invDxDx, tripletList, b,
                    linearIndex2UnknownIndex[ linearIndex ] );
    linearIndex = input->ComputeOffset( indexXM );
    this->UpdateAB( i, indexXM, invDxDx, tripletList, b,
                    linearIndex2UnknownIndex[ linearIndex ] );
    linearIndex = input->ComputeOffset( indexYP );
    this->UpdateAB( i, indexYP, invDyDy, tripletList, b,
                    linearIndex2UnknownIndex[ linearIndex ] );
    linearIndex = input->ComputeOffset( indexYM );
    this->UpdateAB( i, indexYM, invDyDy, tripletList, b,
                    linearIndex2UnknownIndex[ linearIndex ] );
    linearIndex = input->ComputeOffset( indexZP );
    this->UpdateAB( i, indexZP, invDzDz, tripletList, b,
                    linearIndex2UnknownIndex[ linearIndex ] );
    linearIndex = input->ComputeOffset( indexZM );
    this->UpdateAB( i, indexZM, invDzDz, tripletList, b,
                    linearIndex2UnknownIndex[ linearIndex ] );
    }

  itkDebugMacro( << "Done building linear systems" );

  // Fill A
  A.setFromTriplets( tripletList.begin(), tripletList.end() );
  tripletList.clear();

  // Compress the matrix before handing it off to the solver
  A.makeCompressed();

  itkDebugMacro( << "Done compressing" );

  std::time_t startTime;
  std::time( &startTime );

  // Tried several solvers on one large image to find the fastest:
  // ConjugateGradient  - 111 seconds
  // SimplicialCholesky - 204 seconds
  // SimplicialLDLT     - 203 seconds
  // SimplicalLLT       - error
  Eigen::ConjugateGradient< MatrixType > solver;
  solver.compute( A );
  VectorType x = solver.solve( b );

  itkDebugMacro( << "Done solving" );

  std::time_t endTime;
  std::time( &endTime );

  double elapsedSeconds = std::difftime( endTime, startTime );
  (void) elapsedSeconds; // To avoid warning when compiled in release mode
  itkDebugMacro( << "Execution took " << elapsedSeconds << " seconds." );

  if ( solver.info() != Eigen::Success )
    {
    itkExceptionMacro( << "Failed to solve linear system." );
    }

  // Now copy the output vector to the output image
  for ( size_t i = 0; i < solutionIndices.size(); ++i )
    {
    output->SetPixel( solutionIndices[i], x[i] );
    }

  // Now set the values for the Dirichlet boundary conditions
  // in the output image.
  ImageRegionConstIterator< TInputImage > iterIn( input.GetPointer(),
                                                     input->GetBufferedRegion() );
  ImageRegionIterator< TOutputImage > iterOut( output.GetPointer(),
                                                  output->GetBufferedRegion() );

  while ( !iterIn.IsAtEnd() && !iterOut.IsAtEnd() )
    {
    typename DirichletMapType::iterator iter = m_DirichletBoundaryConditionMap.find( iterIn.Get() );
    if ( iter != m_DirichletBoundaryConditionMap.end() )
      {
      iterOut.Set( iter->second );
      }
    ++iterIn;
    ++iterOut;
    }
}

} // end namespace itk

#endif
