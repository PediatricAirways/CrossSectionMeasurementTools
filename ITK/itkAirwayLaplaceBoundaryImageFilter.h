#ifndef itkLaplaceBoundaryImageFilter_h_included
#define itkLaplaceBoundaryImageFilter_h_included

#include <itkImageToImageFilter.h>
#include <itkPoint.h>



#include <vector>


namespace itk
{

/** \class AirwayLaplaceBoundaryImageFilter
 * \brief Labels the boundary of an airway with the boundary
 *        conditions for the Laplace equation.
 *
 * Authors: Schuyler Kylstra. */
template< typename TInputImage >
class AirwayLaplaceBoundaryImageFilter: public ImageToImageFilter< TInputImage, TInputImage >
{
public:
  /** Standard class typedefs. */
  typedef AirwayLaplaceBoundaryImageFilter                Self;
  typedef ImageToImageFilter< TInputImage, TInputImage >  Superclass;
  typedef SmartPointer< Self >                            Pointer;
  typedef SmartPointer< const Self >                      ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Runtime information support. */
  itkTypeMacro(AirwayLaplaceBoundaryImageFilter, ImageToImageFilter);

  typedef typename TInputImage::PointType PointType;


  itkSetMacro( NasalPoint, PointType );
  itkGetMacro( NasalPoint, PointType );

  itkSetMacro( NasalVector, PointType );
  itkGetMacro( NasalVector, PointType );

  itkSetMacro( TracheaPoint, PointType );
  itkGetMacro( TracheaPoint, PointType );

  itkSetMacro( TracheaVector, PointType );
  itkGetMacro( TracheaVector, PointType );

  typedef          TInputImage            InputImageType;
  typedef typename TInputImage::PixelType InputPixelType;
  typedef typename TInputImage::IndexType InputIndexType;
  typedef typename TInputImage::PixelType OutputPixelType;
  typedef typename TInputImage::IndexType OutputIndexType;


  /** Set/get the voxels that are part of the interior of the airway */
  itkSetMacro( InteriorAirwayBoundaryConditionLabel, InputPixelType );
  itkGetMacro( InteriorAirwayBoundaryConditionLabel, InputPixelType );

  /** Set/get the label used to designate a zero-flux Neumann boundary
   *  condition. */
  itkSetMacro( NeumannBoundaryConditionLabel, InputPixelType );
  itkGetMacro( NeumannBoundaryConditionLabel, InputPixelType );


  /* Set the boundary condition at the nose plane */
  itkSetMacro( NostrilBoundaryConditionLabel, InputPixelType );
  itkGetMacro( NostrilBoundaryConditionLabel, InputPixelType );


  /* Set the boundary condition at the Trachea Charina */ 
  itkSetMacro( TracheaBoundaryConditionLabel, InputPixelType );
  itkGetMacro( TracheaBoundaryConditionLabel, InputPixelType );


protected:
  AirwayLaplaceBoundaryImageFilter();
  virtual ~AirwayLaplaceBoundaryImageFilter() {}

  /**
   * Standard pipeline method.
   */
  void GenerateData();

private:
  AirwayLaplaceBoundaryImageFilter(const Self &); //purposely not implemented
  void operator=(const Self &); //purposely not implemented

  InputPixelType m_InteriorAirwayBoundaryConditionLabel;
  InputPixelType m_NostrilBoundaryConditionLabel;
  InputPixelType m_TracheaBoundaryConditionLabel;
  InputPixelType m_NeumannBoundaryConditionLabel;
  PointType m_NasalPoint, m_NasalVector;
  PointType m_TracheaPoint, m_TracheaVector;

  void DefineBoundary();
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkAirwayLaplaceBoundaryImageFilter.hxx"
#endif


#endif
