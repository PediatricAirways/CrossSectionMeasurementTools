#ifndef itkLaplaceSolutionFilter_h_included
#define itkLaplaceSolutionFilter_h_included

#include <itkImageToImageFilter.h>
#include <itkPoint.h>
#include "Boundary/itkAirwayLaplaceBoundaryImageFilter.h"
#include "Solver/itkLaplaceEquationSolverImageFilter.h"



#include <vector>


namespace itk
{

/** \class AirwayLaplaceBoundaryImageFilter
 * \brief Labels the boundary of an airway with the boundary
 *        conditions for the Laplace equation.
 *
 * Authors: Schuyler Kylstra. */
template< typename TInputImage, typename TOutputImage >
class AirwayLaplaceSolutionFilter: public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef AirwayLaplaceSolutionFilter 		              Self;
  typedef ImageToImageFilter< TInputImage, TOutputImage > Superclass;
  typedef SmartPointer< Self >                            Pointer;
  typedef SmartPointer< const Self >                      ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Runtime information support. */
  itkTypeMacro(AirwayLaplaceBoundaryImageFilter, ImageToImageFilter);

  typedef typename TInputImage::PointType PointType;


  itkSetMacro( NosePoint, PointType );
  itkGetMacro( NosePoint, PointType );

  itkSetMacro( NoseVector, PointType );
  itkGetMacro( NoseVector, PointType );

  itkSetMacro( TrachPoint, PointType );
  itkGetMacro( TrachPoint, PointType );

  itkSetMacro( TrachVector, PointType );
  itkGetMacro( TrachVector, PointType );


  typedef typename TInputImage::PixelType   InputPixelType;
  typedef typename TInputImage::IndexType   InputIndexType;
  typedef typename TOutputImage::PixelType  OutputPixelType;
  typedef typename TOutputImage::IndexType  OutputIndexType;


protected:
  AirwayLaplaceSolutionFilter();
  virtual ~AirwayLaplaceSolutionFilter() {}

  /**
   * Standard pipeline method.
   */
  void GenerateData();

private:
  AirwayLaplaceSolutionFilter(const Self &); //purposely not implemented
  void operator=(const Self &); //purposely not implemented

  PointType m_NosePoint, m_NoseVector;
  PointType m_TrachPoint, m_TrachVector;

  void DefineBoundary();
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkAirwayLaplaceSolutionFilter.hxx"
#endif

#endif
