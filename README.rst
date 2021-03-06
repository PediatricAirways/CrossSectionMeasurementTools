Cross Section Measurement Tools
===============================

This is a set of command-line executables used to process CT scans of
the upper airway. Major components include:

* ConvertDICOMTONRRD - a DICOM-to-NRRD file converter that resamples
  images to an orthgonal grid aligned with the major axes.

* ComputeLaplaceSolution - a utility to compute the Laplace solution
  to the heat flow equation through a segmented object initialized
  with temperature boundary conditions. This can be used to compute a
  centerline through a tube-like object.

* ComputeHeatContours - a utility to compute isocontours through a heat
  image produced by ComputeLaplaceSolution. These are used to determine
  the airway centerline and tangent.

* ComputeLBMBoundaries - a utility to compute an image with inflow and
  outflow boundary conditions needed by Sorin Mitran's
  Lattice-Boltzmann fluid solver.

* ConvertPolyDataToImage - given a VTK polygonal mesh file, converts
  it to a binary image where voxels inside the mesh have one value and
  voxels outside the mesh have another value.

* ConvertSpheres - utility that converts ParaView state files with
  spherical Clip filters (used to clip away air in the mouth that is
  included with the segmentation) to a simple text format.

* ExtractCrossSections - given a full set of cross sections, extracts
  those nearest a set of given points.

* RemoveSphere - clips an image file and VTK polygonal data file by a
  sphere of a given size.

* ResampleImage - resamples an image to a given size and spacing.
  The interpolation method can be set at run time.

* SplitEpiglottisCrossSection - reads files produced by
  ExtractCrossSections and a landmark file to split a cross section
  associated with the epiglottis tip into two components, one that is
  front of the epiglottis tip and one that is behind. Writes these
  different components to separate VTP files and also writes a small
  text file with the area of the front section on the first line and
  the area of the back section on the second line.

* ThresholdLaplaceSolution - given a heat flow image generated by
  ComputeLaplaceSolution, thesholds only the valid region.

How to use the Cross Section Measurement Tools
----------------------------------------------

The easiest way to use these tools is via the workflow defined in the
Workflow directory. To use it, set your PYTHONPATH to include the
directory to the steady package (available at
https://github.com/KitwareMedical/steady). Modify the paths at the top
of PediatricAirwaysWorkflow.py to point to your local AirwaysDatabase.
Then run

python PediatricAirwaysWorkflow.py <scan ID>

where <scan ID> is the ID of the scan.

This will segment the airway and run cross section analysis on the
resulting segmentation.

Prerequisites
-------------

* VTK with the VTK_Group_Imaging module ON

* ITK with Module_ITKVtkGlue ON and VTK_DIR set to the VTK build directory

* SlicerExecutionModel

* AirwaySegmenter (https://github.com/PediatricAirways/AirwaySegmenter)
