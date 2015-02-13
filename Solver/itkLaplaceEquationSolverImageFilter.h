#ifndef itkLaplaceEquationSolverImageFilter_h_included
#define itkLaplaceEquationSolverImageFilter_h_included

#include <itkImageToImageFilter.h>

#include <Eigen/Sparse>

#include <vector>


namespace itk
{

/** \class LaplaceEquationSolverImageFilter
 * \brief Computes the solution to the Laplace equation for two
 * fixed (Dirichlet) and one Neumann (zero-flux) boundary conditions.
 *
 * This filter can be used to compute a smooth scalar field
 * transitioning from one surface to another within a solution
 * region. This can be useful to, for instance, compute cross sections
 * of a tube-like objects.
 *
 * Authors: Marc Niethammer and Yi Hong wrote MATLAB code that was
 * adapted to ITK by Cory Quammen. */
template< typename TInputImage, typename TOutputImage >
class LaplaceEquationSolverImageFilter: public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef LaplaceEquationSolverImageFilter                Self;
  typedef ImageToImageFilter< TInputImage, TOutputImage > Superclass;
  typedef SmartPointer< Self >                            Pointer;
  typedef SmartPointer< const Self >                      ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Runtime information support. */
  itkTypeMacro(LaplaceEquationSolverImageFilter, ImageToImageFilter);

  typedef typename TInputImage::PixelType  InputPixelType;
  typedef typename TInputImage::IndexType  InputIndexType;
  typedef typename TOutputImage::PixelType OutputPixelType;
  typedef typename TOutputImage::IndexType OutputIndexType;

  /** Set/get the label used to designate a voxel for which the
   *  Laplace equation solution should be computed. */
  itkSetMacro( SolutionLabel, InputPixelType );
  itkGetMacro( SolutionLabel, InputPixelType );

  /** Set/get the label used to designate a zero-flux Neumann boundary
   *  condition. */
  itkSetMacro( NeumannBoundaryConditionLabel, InputPixelType );
  itkGetMacro( NeumannBoundaryConditionLabel, InputPixelType );

  /** Clear the Dirichlet boundary conditions. */
  void ResetDirichletBoundaryConditions();

  /** Add a Dirichlet boundary condition.
   *
   * This maps voxels in the input image with the value
   * boundaryConditionLabel to the value boundaryConditionValue in the
   * output image. This method must be called at least once to get a
   * reasonable output from this filter. */
  void AddDirichletBoundaryCondition( InputPixelType boundaryConditionLabel,
                                      OutputPixelType boundaryConditionValue );

  /** Get the number of Dirichlet boundary conditions. */
  size_t GetNumberOfDirichletBoundaryConditions() const;

// #ifdef ITK_USE_CONCEPT_CHECKING
//   // Begin concept checking
//   itkConceptMacro( FloatTypeCheck,
//                    ( Concept::FloatOrDouble< OutputPixelType > ) );
//   // End concept checking
// #endif

protected:
  LaplaceEquationSolverImageFilter();
  virtual ~LaplaceEquationSolverImageFilter() {}

  /**
   * Standard pipeline method.
   */
  void GenerateData();

private:
  LaplaceEquationSolverImageFilter(const Self &); //purposely not implemented
  void operator=(const Self &); //purposely not implemented

  InputPixelType m_SolutionLabel;
  InputPixelType m_NeumannBoundaryConditionLabel;

  typedef std::map< InputPixelType, OutputPixelType > DirichletMapType;
  DirichletMapType m_DirichletBoundaryConditionMap;

  typedef std::vector< Eigen::Triplet< OutputPixelType > >    TripletListType;
  typedef Eigen::Matrix< OutputPixelType, Eigen::Dynamic, 1 > VectorType;
  void UpdateAB( size_t i, InputIndexType index, OutputPixelType invDxDx,
                 TripletListType & tripletList, VectorType & b,
                 OffsetValueType solutionIndex );
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkLaplaceEquationSolverImageFilter.hxx"
#endif

#endif
