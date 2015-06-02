#ifndef itkRasterizeSphereImageFilter_hxx_included
#define itkRasterizeSphereImageFilter_hxx_included

#include "itkImage.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkObjectFactory.h"
#include "itkRasterizeSphereImageFilter.h"

#include <cstdio>

namespace itk
{
template<class ImageType>
RasterizeSphereImageFilter<ImageType>
::RasterizeSphereImageFilter()
{
  this->m_SphereRadius = 0.0;
}


template<class ImageType>
void RasterizeSphereImageFilter<ImageType>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType itkNotUsed(threadId))
{
  typename ImageType::ConstPointer input = this->GetInput();
  typename ImageType::Pointer output = this->GetOutput();

  typename ImageType::PointType voxelPoint;
  typename ImageType::PixelType voxelValue;

  itk::ImageRegionIteratorWithIndex<ImageType> out(output, outputRegionForThread);
  itk::ImageRegionConstIteratorWithIndex<ImageType> it(input, outputRegionForThread);

  out.GoToBegin();
  it.GoToBegin();

  double dist;
  double radius2 = m_SphereRadius*m_SphereRadius;

  while(!out.IsAtEnd())
  {
    voxelValue = it.Value();
    input->TransformIndexToPhysicalPoint( it.GetIndex() , voxelPoint );
    dist = 0;
    for( unsigned int i = 0; i < ImageType::ImageDimension; ++i)
    {
      dist += (voxelPoint[i]-m_SphereCenter[i])*(voxelPoint[i]-m_SphereCenter[i]);
    }

    if (dist < radius2)
    {
      voxelValue = 0;
    }

    out.Set(voxelValue);

    ++it;
    ++out;
  }

}

}// end namespace

#endif //itkRasterizeSphereImageFilter_hxx_included
