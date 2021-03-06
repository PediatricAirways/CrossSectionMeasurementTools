project(ComputeCrossSections)
cmake_minimum_required(VERSION 2.8.9)

set(MODULE_NAME ComputeCrossSections)

find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

find_package(ITK 4.7 REQUIRED)
include(${ITK_USE_FILE})

# Include VTK
find_package( VTK REQUIRED )
include(${VTK_USE_FILE})

# Slicer doesn't enable ITK's VtkGlue module, so we add the include
# directory manually here.
load_cache( "${ITK_DIR}" READ_WITH_PREFIX My ITK_SOURCE_DIR )
include_directories( ${MyITK_SOURCE_DIR}/Modules/Bridge/VtkGlue/include
  ${CMAKE_CURRENT_SOURCE_DIR})

### CLI module that extracts the cross sections closest to a set of
### query points and saves them as a poly data.
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  ADDITIONAL_SRCS
    vtkContourCompleter.h
    vtkContourCompleter.cxx
  TARGET_LIBRARIES
    ${ITK_LIBRARIES}
    ${VTK_LIBRARIES}
  EXECUTABLE_ONLY
  RUNTIME_OUTPUT_DIRECTORY ${MODULE_RUNTIME_OUTPUT_DIRECTORY}
)
