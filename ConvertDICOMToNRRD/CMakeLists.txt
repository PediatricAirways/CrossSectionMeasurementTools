cmake_minimum_required(VERSION 2.8.9)
if(NOT DEFINED CMAKE_MACOSX_RPATH)
  set(CMAKE_MACOSX_RPATH OFF)
endif()

project(DicomToNrrd)

# Find packages
find_package(ITK 4 REQUIRED)
include(${ITK_USE_FILE})

find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

find_package(VTK REQUIRED)

# Needed because ITK will clobber the VTK_LIBRARIES and VTK_INCLUDE_DIRS
vtk_module_config(ADDITIONAL_VTK vtkIOXML)
include(${VTK_USE_FILE})

option( SUPPORT_UCHAR_PIXEL "Build with support for input images with unsigned char pixels." OFF )
option( SUPPORT_CHAR_PIXEL "Build with support for input images with char pixels." OFF )
option( SUPPORT_USHORT_PIXEL "Build with support for input images with unsigned short pixels." ON )
option( SUPPORT_SHORT_PIXEL "Build with support for input images with short pixels." ON )
option( SUPPORT_UINT_PIXEL "Build with support for input images with unsigned int pixels." OFF )
option( SUPPORT_INT_PIXEL "Build with support for input images with int pixels." OFF )
option( SUPPORT_ULONG_PIXEL "Build with support for input images with unsigned long pixels." OFF )
option( SUPPORT_LONG_PIXEL "Build with support for input images with long pixels." OFF )
option( SUPPORT_FLOAT_PIXEL "Build with support for input images with float pixels." OFF )
option( SUPPORT_DOUBLE_PIXEL "Build with support for input images with double pixels." OFF )

#Compatibility between slicer and the executable
set(SLICER_ITK_DIFFERENT true CACHE BOOL "Check if compiling with an ITK build different from the one used by Slicer")
configure_file(${DicomToNrrd_SOURCE_DIR}/DicomToNrrdConfig.h.in
               ${DicomToNrrd_BINARY_DIR}/DicomToNrrdConfig.h)

include_directories(
  ${DicomToNrrd_SOURCE_DIR}
  ${DicomToNrrd_SOURCE_DIR}/Code
  ${ADDITIONAL_VTK_INCLUDE_DIRS}
)

set(CMAKE_SHARED_LINKER_FLAGS -DDLL_EXPORT)

#Finally the projects
SEMMacroBuildCLI(
  NAME ${PROJECT_NAME}
  TARGET_LIBRARIES ${ITK_LIBRARIES}
                   ${VTK_LIBRARIES}
  EXECUTABLE_ONLY
  RUNTIME_OUTPUT_DIRECTORY ${MODULE_RUNTIME_OUTPUT_DIRECTORY}
)