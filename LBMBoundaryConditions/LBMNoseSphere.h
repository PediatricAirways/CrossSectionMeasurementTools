#ifndef LBMNoseSphere_h_included
#define LBMNoseSphere_h_included

namespace {

template< class TBinaryImage >
bool
InsideSphere( typename TBinaryImage::IndexType index,
              TBinaryImage * binaryImage,
              typename TBinaryImage::PointType sphereCenter,
              double sphereRadius );


template< class TBinaryImage >
bool
OnSphereEdge( typename TBinaryImage::IndexType index,
              TBinaryImage * binaryImage,
              typename TBinaryImage::PointType sphereCenter,
              double sphereRadius );


template< class TBinaryImage >
typename TBinaryImage::RegionType
GetNoseSphereRegion( typename TBinaryImage::PointType sphereCenter,
                     double sphereRadius,
                     const TBinaryImage * binaryImage );


template< class TBinaryImage, class TOriginalImage >
void
AddNoseSphere( typename TBinaryImage::PointType sphereCenter,
               double sphereRadius,
               TBinaryImage * binaryImage,
               TOriginalImage * originalImage,
               double airThreshold,
               typename TBinaryImage::PixelType interiorValue,
               typename TBinaryImage::PixelType exteriorValue,
               typename TBinaryImage::PixelType inflowValue );

} // end anonymous namespace

#include "LBMNoseSphere.hxx"

#endif
