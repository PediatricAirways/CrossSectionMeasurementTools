project(ResampleImage)
cmake_minimum_required(VERSION 2.8)

#-----------------------------------------------------------------------------
set(MODULE_NAME ResampleImage)

#-----------------------------------------------------------------------------
find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

find_package(VTK)
include( ${VTK_USE_FILE} )

find_package(ITK)
include( ${ITK_USE_FILE} )

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  TARGET_LIBRARIES
        ${VTK_LIBRARIES} ${ITK_LIBRARIES}
  EXECUTABLE_ONLY
  RUNTIME_OUTPUT_DIRECTORY ${MODULE_RUNTIME_OUTPUT_DIRECTORY}
)
