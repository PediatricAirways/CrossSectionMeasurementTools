project(LaplaceSolution)
cmake_minimum_required(VERSION 2.8)

find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

find_package(ITK 4.7
  COMPONENTS
    ITKIOImageBase
    ITKVTK
 REQUIRED)
include(${ITK_USE_FILE})

# Include Eigen3
find_package( Eigen3 REQUIRED )
include_directories( ${EIGEN3_INCLUDE_DIR} )

add_executable( TestHeatFlow TestHeatFlow.cxx )
target_link_libraries( TestHeatFlow ${ITK_LIBRARIES} )

add_definitions( -D_SCL_SECURE_NO_WARNINGS )
