cmake_minimum_required(VERSION 2.8.9)

project(ExcludeSphere)

#-----------------------------------------------------------------------------
set(MODULE_NAME ExcludeSphere)

#-----------------------------------------------------------------------------
find_package(Eigen3 REQUIRED)

#-----------------------------------------------------------------------------
find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

#-----------------------------------------------------------------------------
find_package(ITK 4.7 REQUIRED)
include( ${ITK_USE_FILE} )

#-----------------------------------------------------------------------------
find_package(VTK REQUIRED)
include( ${VTK_USE_FILE} )

#-----------------------------------------------------------------------------
set(MODULE_INCLUDE_DIRECTORIES
  ${EIGEN3_INCLUDE_DIR}
  )

set(MODULE_TARGET_LIBRARIES
  ${ITK_LIBRARIES}
  ${VTK_LIBRARIES}
  )

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  TARGET_LIBRARIES ${MODULE_TARGET_LIBRARIES}
  INCLUDE_DIRECTORIES ${MODULE_INCLUDE_DIRECTORIES}
  EXECUTABLE_ONLY
  RUNTIME_OUTPUT_DIRECTORY ${MODULE_RUNTIME_OUTPUT_DIRECTORY}
  )

