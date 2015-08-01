import os
import sys

# Add Python path two levels up
sys.path.append('../..')

import glob
from steady import workflow as wf

executablePath  = '/home/cory/code/bin/CrossSectionMeasurementTools/bin/'
segmenterPath   = '/home/cory/code/bin/AirwaySegmenter/bin'
rootPath        = '/home/cory/AirwaysDatabase'
python          = '/usr/bin/python'
unzip           = '/usr/bin/unzip'

#############################################################################
def AreNeededFilesPresent(scanId):
    # Check that all needed files are present
    neededFiles = ['_LOWER_CUTOFF.fcsv',
                   '_LOWER.fcsv',
                   '.mrml',
                   '_NOSE_SPHERE.fcsv',
                   '_UPPER.fcsv',
                   '_LANDMARKS.fcsv']

    print('Checking that needed files are present...')
    allFilesPresent = True
    for file in neededFiles:
        fileName = os.path.join(rootPath, scanId, scanId + file)
        if (not os.path.isfile(fileName)):
            sys.stderr.write( 'Required file "%s" not present\n' % fileName )
            allFilesPresent = False

    return allFilesPresent

#############################################################################
def AddUnzipDICOMsStep(pipeline, scanId):
    root = os.path.join(rootPath, scanId, scanId)
    dicomZip = root + '_DICOMS.zip'
    dicomDirectory = root + '_DICOMS/'
    dicomDirectoryParent = os.path.dirname(dicomDirectory[:-1])

    cmd = [unzip, '-d', dicomDirectoryParent, wf.outfile_hidden(dicomDirectory), '-u', wf.infile(dicomZip)]
    unzipStep = wf.CLIWorkflowStep('UnzipDICOMs-' + scanId, cmd)

    pipeline.AddStep(unzipStep)

#############################################################################
def AddDICOMToNRRDStep(pipeline, scanId):
    # Dicom to Nrrd
    root = os.path.join(rootPath, scanId, scanId)
    converterExe = os.path.join(executablePath, 'ConvertDICOMToNRRD')

    cmd = [python,
           wf.infile('ConvertDICOMToNRRD.py'),
           wf.infile(converterExe),
           wf.infile(root + '_DICOMS/'),
           wf.outfile(root + '_INPUT.nrrd')]
    conversionStep = wf.CLIWorkflowStep('DICOMToNRRD-' + scanId, cmd)

    pipeline.AddStep(conversionStep)

#############################################################################
def AddLinkFragmentsStep(pipeline, scanId):
    # Add link fragment step
    root = os.path.join(rootPath, scanId, scanId)
    linksFilePath = root + '_LINKS.fcsv'

    if (os.path.isfile(linksFilePath)):
        linksFilePath = wf.infile(linksFilePath)

    cmd = [python,
           wf.infile('LinkFragments.py'),
           wf.infile(os.path.join(segmenterPath, 'DrawLines')),
           linksFilePath,
           wf.infile(root + '_INPUT.nrrd'),
           wf.outfile(root + '_INPUT_LINKED.nrrd')]
    linkFragmentsStep = wf.CLIWorkflowStep('LinkFragments-' + scanId, cmd)

    pipeline.AddStep(linkFragmentsStep)

#############################################################################
def AddSegmentAirwayStep(pipeline, scanId):
    # Segmentation step
    root = os.path.join(rootPath, scanId, scanId)

    cmd = [python,
           wf.infile('SegmentAirway.py'),
           wf.infile(os.path.join(segmenterPath, 'AirwaySegmenter')),
           wf.infile(root + '_LOWER.fcsv'),
           wf.infile(root + '_UPPER.fcsv'),
           wf.infile(root + '.mrml'),
           wf.infile(root + '_INPUT_LINKED.nrrd'),
           wf.outfile(root + '_OUTPUT.mha'),
           wf.outfile(root + '_OUTPUT.vtp'),
           wf.outfile(root + '_THRESHOLD.txt')]
    segmentationStep = wf.CLIWorkflowStep('SegmentAirway-'+ scanId, cmd)

    pipeline.AddStep(segmentationStep)

#############################################################################
def AddExtractSpheresStep(pipeline, scanId):
    # Extract mouth removal spheres if XXXX_CLIPPINGS.pvsm is available
    root = os.path.join(rootPath, scanId, scanId)
    clippingsFilePath = root + '_CLIPPINGS.pvsm'
    if (os.path.isfile(clippingsFilePath)):

        cmd = [python,
               wf.infile('ExtractSpheres.py'),
               wf.infile(clippingsFilePath),
               wf.outfile(root + '_CLIPPINGS.txt')]
        defineSpheresStep = wf.CLIWorkflowStep('ExtractSpheres-' + scanId, cmd)

        pipeline.AddStep(defineSpheresStep)

#############################################################################
def AddRemoveMouthStep(pipeline, scanId):
    # Mouth removal step
    root = os.path.join(rootPath, scanId, scanId)

    clippingsFilePath = root + '_CLIPPINGS.txt'

    clippingsFilePath = wf.infile(clippingsFilePath)

    cmd = [python,
           wf.infile('RemoveMouth.py'),
           wf.infile(os.path.join(executablePath, 'RemoveSphere')),
           wf.infile(root + '_OUTPUT.mha'),
           wf.infile(root + '_OUTPUT.vtp'),
           clippingsFilePath,
           wf.outfile(root + '_MOUTH_REMOVED.mha'),
           wf.outfile(root + '_MOUTH_REMOVED.vtp')]
    mouthRemovalStep = wf.CLIWorkflowStep('RemoveMouth-' + scanId, cmd)

    pipeline.AddStep(mouthRemovalStep)

#############################################################################
def AddComputeLaplaceSolutionStep(pipeline, scanId):
    # Laplace solution step
    root = os.path.join(rootPath, scanId, scanId)

    cmd = [python,
           wf.infile('ComputeLaplaceSolution.py'),
           wf.infile(os.path.join(executablePath, 'ComputeLaplaceSolution')),
           wf.infile(root + '_MOUTH_REMOVED.mha'),
           wf.infile(root + '_LANDMARKS.fcsv'),
           wf.outfile(root + '_HEATFLOW.mha')]
    laplaceCalculationStep = wf.CLIWorkflowStep('LaplaceCalculation-' + scanId, cmd)

    pipeline.AddStep(laplaceCalculationStep)

#############################################################################
def AddThresholdLaplaceSolutionStep(pipeline, scanId):
    # Threshold the heatflow solution to only valid range [0, 1]
    root = os.path.join(rootPath, scanId, scanId)
    cmd = [os.path.join(executablePath, 'ThresholdLaplaceSolution'),
           wf.infile(root + '_HEATFLOW.mha'),
           wf.outfile(root + '_HEATFLOW_THRESHOLDED.vtu')]

    pipeline.AddStep(wf.CLIWorkflowStep('ThresholdLaplaceSolution-' + scanId, cmd))

#############################################################################
def AddComputeHeatContoursStep(pipeline, scanId):
    # Compute contours through the valid heatflow region
    root = os.path.join(rootPath, scanId, scanId)
    cmd = [os.path.join(executablePath, 'ComputeHeatContours'),
           wf.infile(root + '_HEATFLOW_THRESHOLDED.vtu'),
           wf.outfile(root + '_HEATFLOW_CROSS_SECTIONS.vtp')]

    pipeline.AddStep(wf.CLIWorkflowStep('ComputeHeatContours-' + scanId, cmd))

#############################################################################
def AddComputeCrossSectionsStep(pipeline, scanId):
    # Full cross-section computation step
    root = os.path.join(rootPath, scanId, scanId)
    cmd = [os.path.join(executablePath, 'ComputeCrossSections'),
           wf.infile(root + '_HEATFLOW_CROSS_SECTIONS.vtp'),
           wf.infile(root + '_MOUTH_REMOVED.vtp'),
           wf.outfile(root + '_ALL_CROSS_SECTIONS.vtp'),
           wf.outfile(root + '_ALL_CROSS_SECTIONS.csv')]

    pipeline.AddStep(wf.CLIWorkflowStep('CalculateAllCrossSections-' + scanId, cmd))

#############################################################################
def AddExtractLandmarkCrossSectionsStep(pipeline, scanId):
    # Cross-sections for landmarks step
    root = os.path.join(rootPath, scanId, scanId)
    cmd = [python,
           wf.infile('ExtractLandmarkCrossSections.py'),
           wf.infile(os.path.join(executablePath, 'ExtractCrossSections')),
           wf.infile(os.path.join(executablePath, 'ExtractLandmarkSliceIndices')),
           wf.infile(root + '_ALL_CROSS_SECTIONS.vtp'),
           wf.infile(root + '_ALL_CROSS_SECTIONS.csv'),
           wf.infile(root + '_LANDMARKS.fcsv'),
           wf.outfile(root + '_CROSS_SECTIONS_AT_LANDMARKS.vtp'),
           wf.outfile(root + '_CROSS_SECTIONS_AT_LANDMARKS.csv'),
           scanId]
    extractLandmarkCrossSections = wf.CLIWorkflowStep('ExtractLandmarkCrossSections-' + scanId, cmd)

    pipeline.AddStep(extractLandmarkCrossSections)

#############################################################################
def AddSplitEpiglottisCrossSectionStep(pipeline, scanId):
    # Split the EpiglottisTip cross section by a anterior-posterior
    # oriented cutting plane to record the areas of this cross section
    # in front of and behind the tip.
    root = os.path.join(rootPath, scanId, scanId)
    cmd = [os.path.join(executablePath, 'SplitEpiglottisCrossSection'),
           wf.infile(root + '_CROSS_SECTIONS_AT_LANDMARKS.vtp'),
           wf.infile(root + '_LANDMARKS.fcsv'),
           wf.outfile(root + '_EPIGLOTTIS_TIP_AREAS.txt'),
           wf.outfile(root + '_EPIGLOTTIS_FRONT.vtp'),
           wf.outfile(root + '_EPIGLOTTIS_BACK.vtp')]
    splitEpiglottisCS = wf.CLIWorkflowStep('SplitEpiglottisCrossSection-' + scanId, cmd)

    pipeline.AddStep(splitEpiglottisCS)

#############################################################################
def AddResampleCTStep(pipeline, scanId):
    # Resample CT image to 0.5 mm spacing in each dimension
    root = os.path.join(rootPath, scanId, scanId)
    cmd = [os.path.join(executablePath, 'ResampleImage'),
           '--interpolator', 'BSpline', '--spacing', '0.5,0.5,0.5',
           wf.infile(root + '_INPUT.nrrd'),
           wf.outfile(root + '_INPUT_RESAMPLED.nrrd')]
    resampleCT = wf.CLIWorkflowStep('ResampleCT-' + scanId, cmd)

    pipeline.AddStep(resampleCT)

#############################################################################
def AddResampleSegmentationStep(pipeline, scanId):
    # Resample segmentation image to 0.5 mm spacing in each dimension
    root = os.path.join(rootPath, scanId, scanId)
    cmd = [os.path.join(executablePath, 'ResampleImage'),
           '--interpolator', 'Nearest', '--spacing', '0.5,0.5,0.5',
           wf.infile(root + '_MOUTH_REMOVED.mha'),
           wf.outfile(root + '_SEGMENTATION_RESAMPLED.nrrd')]
    resampleSegmentation = wf.CLIWorkflowStep('ResampleSegmentation-' + scanId, cmd)

    pipeline.AddStep(resampleSegmentation);

#############################################################################
def AddComputeLatticeBoltzmannBoundaryConditionsStep(pipeline, scanId):
    # Lattice Boltzmann boundary condition step
    root = os.path.join(rootPath, scanId, scanId)
    cmd = [python,
           wf.infile('ComputeLatticeBoltzmannBoundaryConditionsStep.py'),
           wf.infile(os.path.join(executablePath, 'ComputeLBMBoundaries')),
           wf.infile(root + '_INPUT_RESAMPLED.nrrd'),
           wf.infile(root + '_SEGMENTATION_RESAMPLED.nrrd'),
           wf.infile(root + '_THRESHOLD.txt'),
           wf.infile(root + '_NOSE_SPHERE.fcsv'),
           wf.infile(root + '_LOWER_CUTOFF.fcsv'),
           wf.outfile(root + '_LBM.vtk')]
    lbmBoundaryConditionStep = wf.CLIWorkflowStep('ComputeLatticeBoltzmannBoundaryCondition-' + scanId, cmd)

    pipeline.AddStep(lbmBoundaryConditionStep)

#############################################################################
def ProcessScan(scanId):
    wf.Workflow.SetCacheDirectory(os.path.join(rootPath, scanId, 'cache'))

    # Set up a pipeline
    pipeline = wf.Workflow()

    AddUnzipDICOMsStep(pipeline, scanId)
    AddDICOMToNRRDStep(pipeline, scanId)
    AddLinkFragmentsStep(pipeline, scanId)
    AddSegmentAirwayStep(pipeline, scanId)
    AddExtractSpheresStep(pipeline, scanId)
    AddRemoveMouthStep(pipeline, scanId)
    AddComputeLaplaceSolutionStep(pipeline, scanId)
    AddThresholdLaplaceSolutionStep(pipeline, scanId)
    AddComputeHeatContoursStep(pipeline, scanId)
    AddComputeCrossSectionsStep(pipeline, scanId)
    AddExtractLandmarkCrossSectionsStep(pipeline, scanId)
    AddSplitEpiglottisCrossSectionStep(pipeline, scanId)
    AddResampleCTStep(pipeline, scanId)
    AddResampleSegmentationStep(pipeline, scanId)
    AddComputeLatticeBoltzmannBoundaryConditionsStep(pipeline, scanId)

    # Options
    forceExecute = '--force-execute' in sys.argv
    dryRun       = '--dryRun' in sys.argv
    verbose      = '--verbose' in sys.argv

    # Clearing the cache forces re-execution of all pipeline steps
    if (forceExecute):
        pipeline.ClearCache()

    if (AreNeededFilesPresent(scanId)):
        pipeline.Execute(dryRun=dryRun, verbose=verbose)

#############################################################################
def main():
    # Create list of directories from glob expression passed in as first argument
    globExpr = os.path.join(rootPath, sys.argv[1])
    dirNames = glob.iglob(globExpr)
    anyScanned = False
    for scan in dirNames:
        print('Processing %s' % scan)
        scanDirectory = os.path.split(scan)[1]
        ProcessScan(scanDirectory)
        anyScanned = True

    if (not anyScanned):
        print('No directories match glob pattern %s' % globExpr)

#############################################################################
if __name__ == '__main__':
    main()
