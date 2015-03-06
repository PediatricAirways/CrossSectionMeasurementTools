class Locations:
	"""docstring for Locations"""
	def __init__(self):
		self.OutputNrrd = '/Users/jonathankylstra/ComputeAirwayCrossSections/Pipeline/Input/OutputNrrd/'
		self.OutputVtk 	= '/Users/jonathankylstra/ComputeAirwayCrossSections/Pipeline/Input/OutputVtk/'
		self.Clippings 	= '/Users/jonathankylstra/ComputeAirwayCrossSections/Pipeline/Input/Clippings/'
		self.Landmarks 	= '/Users/jonathankylstra/ComputeAirwayCrossSections/Pipeline/Input/Landmarks/'

		self.Cross 		= '/Users/jonathankylstra/ComputeAirwayCrossSections/Pipeline/Output/Cross/'
		self.Cut 		= '/Users/jonathankylstra/ComputeAirwayCrossSections/Pipeline/Output/Cut/'
		self.Heatflow 	= '/Users/jonathankylstra/ComputeAirwayCrossSections/Pipeline/Output/Heatflow/'
		self.Segments 	= '/Users/jonathankylstra/ComputeAirwayCrossSections/Pipeline/Output/Segments/'
		self.Data	 	= '/Users/jonathankylstra/ComputeAirwayCrossSections/Pipeline/Output/Data/'

		self.Temp		= '/Users/jonathankylstra/ComputeAirwayCrossSections/Pipeline/temp.csv'
		self.Slices		= '/Users/jonathankylstra/ComputeAirwayCrossSections/Pipeline/Slices.csv'

		self.AllData = '/Users/jonathankylstra/ComputeAirwayCrossSections/Pipeline/AllData.csv'

		# C++ file Locations

		self.ComputeAirwayCrossSections = '/Users/jonathankylstra/Cross-bin/lib/Slicer-4.4/cli-modules'
		self.ExtractCrossSections		= '/Users/jonathankylstra/Cross-bin/lib/Slicer-4.4/cli-modules'
		self.GenericDataObjectReader 	= '/Users/jonathankylstra/DATA/Generic-Build/GenericDataObjectReader.app/Contents/MacOS/'
		self.SliceReader				= '/Users/jonathankylstra/DATA/Slices-Build/SliceReader.app/Contents/MacOS/'
		self.ExcludeSphere				= '/Users/jonathankylstra/ComputeAirwayCrossSections/ExcludeMouth/Build/lib/Slicer-4.4/cli-modules'
		self.AirwayLaplaceSolutionFilter= '/Users/jonathankylstra/heatflowplugin/Build/lib/Slicer-4.4/cli-modules'