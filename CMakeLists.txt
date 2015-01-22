project(vtkCrossSectionMeasurement)
cmake_minimum_required(VERSION 2.8)

# Include VTK
find_package( VTK REQUIRED )
include( ${VTK_USE_FILE} )

# Include ITK
find_package( ITK REQUIRED )
include( ${ITK_USE_FILE} )

add_executable( TestCrossSectionMeasurement
  TestCrossSectionMeasurement.cxx
  vtkCrossSectionImageFilter.cxx
)
target_link_libraries( TestCrossSectionMeasurement ${ITK_LIBRARIES} ${VTK_LIBRARIES} )

add_definitions( -D_SCL_SECURE_NO_WARNINGS )
