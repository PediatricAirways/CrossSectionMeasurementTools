#ifndef itkRasterizeSphereImageFilter_h_included
#define itkRasterizeSphereImageFilter_h_included
 
#include "itkImageToImageFilter.h"
 
namespace itk
{
/** \class RasterizeSphereImageFilter
 *
 * \brief Rasterizes a spherical region in the input image with a given value.
 *
 * \ingroup ImageFilters
 */
template< class ImageType>
class RasterizeSphereImageFilter:public ImageToImageFilter< ImageType, ImageType >
{
public:
  /** Standard class typedefs. */
  typedef typename ImageType::PointType               PointType;
  typedef RasterizeSphereImageFilter                  Self;
  typedef ImageToImageFilter< ImageType, ImageType >  Superclass;
  typedef typename Superclass::OutputImageRegionType  OutputImageRegionType;
  typedef SmartPointer< Self >                        Pointer;
  
 
  /** Method for creation through the object factory. */
  itkNewMacro( Self );
 
  /** Run-time type information (and related methods). */
  itkTypeMacro( RasterizeSphereImageFilter, ImageToImageFilter );

  itkSetMacro( SphereRadius, double );

  itkSetMacro( SphereCenter, PointType );

 
protected:
  RasterizeSphereImageFilter();
  ~RasterizeSphereImageFilter(){}
 
  /** Does the real work. */
  virtual void ThreadedGenerateData( const OutputImageRegionType& outputRegionForThread, ThreadIdType threadId );
 
private:
  RasterizeSphereImageFilter( const Self & ); //purposely not implemented
  void operator=( const Self & );  //purposely not implemented
 
  double                         m_SphereRadius;
  typename ImageType::PointType  m_SphereCenter;
};
} //namespace ITK
 
 
#ifndef ITK_MANUAL_INSTANTIATION
#include "itkRasterizeSphereImageFilter.hxx"
#endif
 
#endif // itkRasterizeSphereImageFilter_h_included
