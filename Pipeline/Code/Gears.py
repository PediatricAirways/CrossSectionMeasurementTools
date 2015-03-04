import os
import subprocess as sub
import Locations as lo



class LaplaceGear:
	"""gear for Pipeline Laplace"""
	def __init__(self):
		pass


#===============================================================================================
#===============================================================================================
#===============================================================================================


class CutGear:
	"""gear for pipeline Cut mouth"""
	def __init__(self):
		pass


#===============================================================================================
#===============================================================================================
#===============================================================================================


class CrossGear:
	""" The Gear that generates the cross sections (pipeCrosssections.py)
	"""
	def __init__(self):
		self.locs = lo.Locations()

	def callFilter(self, D):

		os.chdir("/Users/jonathankylstra/Cross-bin/lib/Slicer-4.4/cli-modules")

		heatfile    = D + "_HEATFLOW.mhd"
		modelFile   = D + "_OUTPUT.vtk"
		output 	    = D + "_CROSS.vtp"

		HEAT_PATH	= os.path.join(self.locs.Heatflow, heatfile)
		MODEL_PATH	= os.path.join(self.locs.OutputVtk, modelFile)
		OUTPUT_PATH	= os.path.join(self.locs.Cross, output)

		print "./ComputeAirwayCrossSections", HEAT_PATH, MODEL_PATH, OUTPUT_PATH
		try:
			sub.call( ["./ComputeAirwayCrossSections", HEAT_PATH, MODEL_PATH, OUTPUT_PATH])
		except Exception, e:
			print e

	def Filter(self, SetOfScanIDs):
		for DIR in SetOfScanIDs:
			try:
				self.callFilter(DIR)
			except Exception, e:
				print e
				continue

#===============================================================================================
#===============================================================================================
#===============================================================================================

class ExtractGear:
	"""gear that runs pipeExtractCrosssections"""
	def __init__(self):
		self.locs = lo.Locations()


	def extractArgs(self, fileobj):
		Subglottic 	= []
		InfSubglot 	= []
		TVC 		= []
		TC 			= []


		for line in fileobj:
			l = line.split(" ")
			if l[0] == "InferiorSubglottis":
				InfSubglot.extend( [float(l[2]),float(l[3]),float(l[4][:-2])] )
			elif l[0] == "Subglottic":
				Subglottic.extend( [float(l[2]),float(l[3]),float(l[4][:-2])] )
			elif l[0] == "TVC":
				TVC.extend( [float(l[2]),float(l[3]),float(l[4][:-2])] )
			elif l[0] == "TracheaCarina":
				TC.extend( [float(l[2]),float(l[3]),float(l[4][:-2])] )
			else:
				pass

		args = []
		args.extend( [TVC, Subglottic, InfSubglot, TC] )
		print "args:", args
		return args



	def callFilter(self, args, D):
		os.chdir("/Users/jonathankylstra/Cross-bin/lib/Slicer-4.4/cli-modules")
		infile = D + "_CROSS.vtp"
		outfile = D + "_ExtractedSegments.vtp"
		IN_PATH=os.path.join(self.locs.Cross, 'Files', infile)
		OUT_PATH=os.path.join(self.locs.Segments, 'Files', outfile)

		tvc = "--queryPoints " + str(args[0][0]) + "," + str(args[0][1]) + "," + str(args[0][2])
		subg = "--queryPoints " + str(args[1][0]) + "," + str(args[1][1]) + "," + str(args[1][2])
		inf = "--queryPoints " + str(args[2][0]) + "," + str(args[2][1]) + "," + str(args[2][2])
		tc = "--queryPoints " + str(args[3][0]) + "," + str(args[3][1]) + "," + str(args[3][2])

		if len(args) == 4:
			try:
				sub.call( ["./ExtractCrossSections", tvc, subg, inf, tc, IN_PATH, OUT_PATH])
			except Exception, e:
				print e
		else:
			mp = "--queryPoints " + str(-args[4][0]) + "," + str(-args[4][1]) + "," + str(args[4][2])
			mt = "--queryPoints " + str(-args[5][0]) + "," + str(-args[5][1]) + "," + str(args[5][2])
			print args
			try:
				sub.call( ["./ExtractCrossSections", tvc, subg, inf, tc, mp, mt, IN_PATH, OUT_PATH])
			except Exception, e:
				print e



	def extractALLData(self, DIR):
		""" Puts all of the slice data into a csv file """

		os.chdir("/Users/jonathankylstra/DATA/Generic-Build/GenericDataObjectReader.app/Contents/MacOS/")
		dirVTP = DIR + "_CROSS.vtp"
		vtpFile = os.path.join(self.locs.Cross, dirVTP)
		tempFile = DIR + '_slices.csv'
		tempFile = os.path.join(self.locs.Segments, tempFile)

		try:
			sub.call( ['./GenericDataObjectReader', vtpFile, tempFile] )
		except Exception, e:
			print e



	def extractData(self, DIR, outfile):
		""" Extracts the information about fiducials specified in extractArgs() """

		os.chdir("/Users/jonathankylstra/DATA/Generic-Build/GenericDataObjectReader.app/Contents/MacOS/")
		dirVTP = DIR + "_ExtractedSegments.vtp"
		vtpFile = os.path.join(self.locs.Segments, dirVTP)


		sub.call( ['./GenericDataObjectReader', vtpFile, self.locs.Temp] )
		f = open(self.locs.Temp, 'r')
		
		lines = list(f)
		l = DIR

		for line in lines[:-1]:
			l =  l + ", " + line[:-1]

		if len(lines) > 0:
			outfile.write(l)

		f.close()




	def GetSliceIndices(self, DIR):
		""" Gets the indices of the TVC and Subglottic slices """

		os.chdir("/Users/jonathankylstra/DATA/Slices-Build/SliceReader.app/Contents/MacOS/")
		dirVTP = DIR + "_ExtractedSegments.vtp"
		vtpFile = os.path.join(self.locs.Segments, dirVTP)
		tempFile = DIR + '_slice.txt'
		tempFile = os.path.join(self.locs.Segments, tempFile)
		sub.call( ['./SliceReader', vtpFile, tempFile] )

		try:
			f = open(tempFile, 'r')
			lines = list(f)
			# sub.call( ['rm', tempFile])
			l = [ int(lines[0][:-1]), int(lines[1][:-1]) , int(lines[2][:-1]) ]
			

			return l

		except Exception, e:
			print e
			return []

	def PlaceMinXASlice(self, inds, DIR, outfile):
		""" Adds information about the min XA to the output file """

		SlicePath = '/Users/jonathankylstra/DATA/' + DIR + '/slices.csv'
		SliceFile = open(SlicePath, 'r')

		lines = list(SliceFile)
		minXA = 1000000
		l = ''
		ind = 0
		for i,line in enumerate(lines[ inds[0] - 1:inds[1] - 1 ]):
			ll = line
			line = line.split(',')
			if float(line[0]) < minXA:
				minXA = float(line[0])
				ind = i + inds[0] 
				l = ll
		outfile.write( ', ' + str(ind) + ', ' + l[:-1] )
		com = l.split(',')[2].split(' ')
		com = [float(com[1]), float(com[2]), float(com[3][:-1])]
		return com


	def PlaceMidTracheaSlice(self, inds, DIR, outfile):
		""" Adds info about MidTrachea """

		SlicePath = '/Users/jonathankylstra/DATA/' + DIR + '/slices.csv'
		SliceFile = open(SlicePath, 'r')

		lines = list(SliceFile)
		
		start = inds[0]
		end   = inds[2]
		

		cLength = [0]
		cumulative = 0

		vals = (lines[start].split(',')[2]).split()
		point0_x = float( vals[0] )
		point0_y = float( vals[1] )
		point0_z = float( vals[2] )

		point1_x = 0
		point1_y = 0
		point1_z = 0

		for i, line in enumerate(lines[start+1:end]):
			vals = (line.split(',')[2]).split()
			point1_x = float( vals[0] )
			point1_y = float( vals[1] )
			point1_z = float( vals[2] )
			length = ( ( point1_x - point0_x )**2 + ( point1_y - point0_y )**2 + ( point1_z - point0_z )**2 )**.5
			cumulative = cumulative + length
			cLength.extend([cumulative])
			point0_x = point1_x
			point0_y = point1_y
			point0_z = point1_z

		MidLength = cLength[-1]/2.0

		i = 0

		while cLength[i] < MidLength:
			i = i + 1

		index = i - 1

		if abs(cLength[i] - MidLength) < abs(cLength[i-1] - MidLength):
			index = i

		l = lines[start + index]
		l = ', ' + l
		outfile.write(l)
		com = l.split(',')[3].split(' ')
		com = [ float(com[1]), float(com[2]), float(com[3][:-1]) ]
		return com




	def Filter(self, SetOfScanIDs):
	for DIR in SetOfScanIDs: 
		landmarks = DIR + "_LANDMARKS.txt"
		try:
			
			
			f = open( os.path.join(self.locs.Landmarks, 'Files', landmarks), 'r' )
			args = extractArgs(f)
			callFilter(args, DIR)
			extractALLData(DIR)
			extractData(DIR, x)
			inds = GetSliceIndices(DIR)
			if len(inds) > 0:

				args.append( PlaceMinXASlice(inds, DIR, x) )
				args.append( PlaceMidTracheaSlice(inds, DIR, x) )

				callFilter(args, DIR)

		except Exception, e:
			print DIR
			print e
			continue


#===============================================================================================
#===============================================================================================
#===============================================================================================


class WriteGear:
	"""gear for Write To CSV"""
	def __init__(self):
		pass
		
		
		