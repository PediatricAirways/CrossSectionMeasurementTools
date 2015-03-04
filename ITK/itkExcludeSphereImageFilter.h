#ifndef itkExcludeSphereImageFilter_h_included
#define itkExcludeSphereImageFilter_h_included
 
#include "itkImageToImageFilter.h"
 
namespace itk
{
/** \class ExcludeSphereImageFilter
 *
 * \ingroup ImageFilters
 */
template< class ImageType>
class ExcludeSphereImageFilter:public ImageToImageFilter< ImageType, ImageType >
{
public:
  /** Standard class typedefs. */
  typedef typename ImageType::PointType               PointType;
  typedef ExcludeSphereImageFilter                    Self;
  typedef ImageToImageFilter< ImageType, ImageType >  Superclass;
  typedef typename Superclass::OutputImageRegionType  OutputImageRegionType;
  typedef SmartPointer< Self >                        Pointer;
  
 
  /** Method for creation through the object factory. */
  itkNewMacro( Self );
 
  /** Run-time type information (and related methods). */
  itkTypeMacro( ExcludeSphereImageFilter, ImageToImageFilter );

  itkSetMacro( SphereRadius, double );

  itkSetMacro( SphereCenter, PointType );

 
protected:
  ExcludeSphereImageFilter();
  ~ExcludeSphereImageFilter(){}
 
  /** Does the real work. */
  virtual void ThreadedGenerateData( const OutputImageRegionType& outputRegionForThread, ThreadIdType threadId );
 
private:
  ExcludeSphereImageFilter( const Self & ); //purposely not implemented
  void operator=( const Self & );  //purposely not implemented
 
  double                         m_SphereRadius;
  typename ImageType::PointType  m_SphereCenter;
};
} //namespace ITK
 
 
#ifndef ITK_MANUAL_INSTANTIATION
#include "itkExcludeSphereImageFilter.hxx"
#endif
 
#endif // itkExcludeSphereImageFilter_h_included
