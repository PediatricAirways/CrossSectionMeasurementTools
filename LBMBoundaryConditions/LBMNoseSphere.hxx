#ifndef LBMNoseSphere_hxx_included
#define LBMNoseSphere_hxx_included

#include <queue>

#include "LBMNoseSphere.h"

namespace {


template< class TBinaryImage >
bool
InsideSphere( typename TBinaryImage::IndexType index,
              TBinaryImage * binaryImage,
              typename TBinaryImage::PointType sphereCenter,
              double sphereRadius )
{
  // Compute distance from voxel to sphere center
  typename TBinaryImage::PointType voxelPosition;
  binaryImage->TransformIndexToPhysicalPoint( index, voxelPosition );
  typename TBinaryImage::PointType::VectorType voxelOffset = voxelPosition - sphereCenter;
  double distance2 = voxelOffset[0]*voxelOffset[0] +
                     voxelOffset[1]*voxelOffset[1] +
                     voxelOffset[2]*voxelOffset[2];

  return ( distance2 <= sphereRadius*sphereRadius );
}


template< class TBinaryImage >
bool
OnSphereEdge( typename TBinaryImage::IndexType index,
              TBinaryImage * binaryImage,
              typename TBinaryImage::PointType sphereCenter,
              double sphereRadius )
{
  // Make sure this voxel is inside the sphere
  if ( !InsideSphere( index, binaryImage, sphereCenter, sphereRadius ) )
    return false;

  bool neighborOutside = false;

  typename TBinaryImage::IndexType nbrIndex;
  typename TBinaryImage::RegionType region = binaryImage->GetLargestPossibleRegion();
  for ( int k = index[2] - 1; k <= index[2] + 1; ++k ) {
    nbrIndex[2] = k;
    for ( int j = index[1] - 1; j <= index[1] + 1; ++j ) {
      nbrIndex[1] = j;
      for ( int i = index[0] - 1; i <= index[0] + 1; ++i ) {
        nbrIndex[0] = i;
        // Check that we're in the image
        if ( region.IsInside( nbrIndex ) ) {
          if ( !InsideSphere( nbrIndex, binaryImage,
                              sphereCenter, sphereRadius ) ) {
            neighborOutside = true;
            break;
          }
        }
      }
    }
  }

  return neighborOutside;
}


template< class TBinaryImage >
typename TBinaryImage::RegionType
GetNoseSphereRegion( typename TBinaryImage::PointType sphereCenter,
                     double sphereRadius,
                     const TBinaryImage * binaryImage )
{
  // Find region bounding the sphere
  typename TBinaryImage::PointType sphereMin( sphereCenter );
  sphereMin -= sphereRadius;
  typename TBinaryImage::IndexType sphereMinIndex;
  binaryImage->TransformPhysicalPointToIndex( sphereMin, sphereMinIndex );

  typename TBinaryImage::PointType sphereMax( sphereCenter );
  sphereMax += sphereRadius;
  typename TBinaryImage::IndexType sphereMaxIndex;
  binaryImage->TransformPhysicalPointToIndex( sphereMax, sphereMaxIndex );

  // Pad min and max index by 1
  for ( int i = 0; i < TBinaryImage::ImageDimension; ++i ) {
    sphereMinIndex[i] -= 1;
    sphereMaxIndex[i] += 1;
  }

  typename TBinaryImage::RegionType sphereRegion;
  sphereRegion.SetIndex( sphereMinIndex );
  sphereRegion.SetUpperIndex( sphereMaxIndex );

  return sphereRegion;
}


template< class TBinaryImage, class TOriginalImage >
void AddNoseSphere( typename TBinaryImage::PointType sphereCenter,
                    double sphereRadius,
                    TBinaryImage * binaryImage,
                    TOriginalImage * originalImage,
                    double airThreshold,
                    typename TBinaryImage::PixelType interiorValue,
                    typename TBinaryImage::PixelType exteriorValue,
                    typename TBinaryImage::PixelType inflowValue )
{
  // Get region of binary image
  typename TBinaryImage::RegionType imageRegion =
    binaryImage->GetLargestPossibleRegion();

  typename TBinaryImage::RegionType sphereRegion =
    GetNoseSphereRegion( sphereCenter, sphereRadius, binaryImage );

  // Crop the sphere region by the image region
  sphereRegion.Crop( imageRegion );

  // Allocate an image to track where we've visited
  const short NOT_VISITED = 0;
  const short SCHEDULED_FOR_VISIT = 1;
  const short VISITED = 2;
  typedef itk::Image< short, 3 > VisitedImageType;
  VisitedImageType::Pointer visitedImage = VisitedImageType::New();
  visitedImage->SetRegions( sphereRegion );
  visitedImage->Allocate();
  visitedImage->FillBuffer( NOT_VISITED );

  // Convert seed point to index in binary image
  typename TBinaryImage::IndexType seedIndex;
  binaryImage->TransformPhysicalPointToIndex( sphereCenter, seedIndex );

  // Push seed index onto stack
  std::queue< typename TBinaryImage::IndexType > queue;
  queue.push( seedIndex );
  visitedImage->SetPixel( seedIndex, SCHEDULED_FOR_VISIT );

  // Process queue elements until queue is empty
  while ( !queue.empty() ) {
    typename TBinaryImage::IndexType index = queue.front();
    queue.pop();

    if ( !imageRegion.IsInside( index ) ) continue;

    typename TBinaryImage::PixelType binaryValue =
      binaryImage->GetPixel( index );
    typename TOriginalImage::PixelType pixelValue =
      originalImage->GetPixel( index );
    typename VisitedImageType::PixelType visited =
      visitedImage->GetPixel( index );

    if ( visited != SCHEDULED_FOR_VISIT ) {
      std::cout << "Voxel " << index << " wasn't scheduled for visit!" << std::endl;
    }

    // If already visited, skip this voxel
    if ( visited == VISITED ) {
      continue;
    }

    // Mark the voxel as visited
    visitedImage->SetPixel( index, VISITED );

    // Compute distance from voxel to sphere center
    bool insideSphere = InsideSphere( index, binaryImage,
                                      sphereCenter, sphereRadius );

    if ( pixelValue <= airThreshold &&
         insideSphere ) {

      // Mark as being in the segmentation
      if ( OnSphereEdge( index, binaryImage, sphereCenter, sphereRadius ) &&
           binaryValue == exteriorValue ) {
        binaryImage->SetPixel( index, inflowValue );
      } else {
        binaryImage->SetPixel( index, interiorValue );
      }

      // Add neighboring pixels to queue if they aren't in the binary
      // image already
      for ( int z = index[2]-1; z <= index[2]+1; ++z ) {
        for ( int y = index[1]-1; y <= index[1]+1; ++y ) {
          for ( int x = index[0]-1; x <= index[0]+1; ++x ) {
            typename TBinaryImage::IndexType nbrIndex = {{ x, y, z }};

            if ( !sphereRegion.IsInside( nbrIndex ) ) continue;

            // Don't add voxels that are already not part of the background
            short visitedStatus = visitedImage->GetPixel( nbrIndex );
            if ( visitedStatus == VISITED ||
                 visitedStatus == SCHEDULED_FOR_VISIT ) continue;

            insideSphere = InsideSphere( nbrIndex, binaryImage,
                                         sphereCenter, sphereRadius );

            // Add only voxels that are within the sphere radius
            if ( insideSphere ) {
              visitedImage->SetPixel( nbrIndex, SCHEDULED_FOR_VISIT );
              queue.push( nbrIndex );
            }
          }
        }
      }
    }
  }
}

} // end anonymous namespace

#endif
