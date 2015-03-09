import os
import subprocess as sub
import Locations as lo
import signal
import numpy as np
from contextlib import contextmanager

class TimeoutException(Exception): pass


@contextmanager
def time_limit(seconds):
    def signal_handler(signum, frame):
        raise TimeoutException, "Timed out!"
    signal.signal(signal.SIGALRM, signal_handler)
    signal.alarm(seconds)
    try:
        yield
    finally:
        signal.alarm(0)

#===============================================================================================
#===============================================================================================
#===============================================================================================


class CutGear:
	"""gear for pipeline Cut mouth"""
	def __init__(self):
		self.locs = lo.Locations()


	def ExtractArgs(self, DIR):
		clips = DIR + "_CLIPPINGS.txt"
		fileobj = open(os.path.join(self.locs.Clippings, 'Files', clips))

		Center 		= []
		Radius		= []

		Center.append([0.0,0.0,0.0])
		Radius.extend([0.0])

		for i,line in enumerate(fileobj):
			l = line.split(" ")
			if i%2 == 0:
				Center.append([-float(l[2]), -float(l[3]), float(l[4])])
			else:
				Radius.extend([float(l[2])])
		args = []
		args.extend( [Center] + [Radius] )
		return args


	def CallFilter(self, args, D):
		os.chdir(self.locs.ExcludeSphere)
		first  	= D + "_OUTPUT.nrrd"
		segfile = D + "_CUT.nrrd"
		FIRST_PATH=os.path.join(self.locs.OutputNrrd, 'Files', first)
		CUT_PATH=os.path.join(self.locs.Cut, 'Files', segfile)

		centers  = args[0]
		radii	 = args[1]
		input_paths = []

		for i,x in enumerate(args[0]):
			if i == 0:
				input_paths.append(str(FIRST_PATH))
			else:
				input_paths.append( str(CUT_PATH) )
			
			inpu 	 = "--input " + input_paths[i]
			output   = "--output " + CUT_PATH
			Center   = "--Center " + str(centers[i][0]) + "," + str(centers[i][1]) + "," + str(centers[i][2])
			Radius	 = "--Radius " + str(radii[i])
			call 	 = "./ExcludeSphere" + ' ' + inpu + ' ' + output + ' ' + Center + ' ' + Radius
			print call
			try:
				sub.call( ["./ExcludeSphere", inpu, output, Center, Radius])
			except Exception, e:
				print e


	def Filter(self, SetOfScanIDs):
		failedScans = self.locs.Cut + "FailedScans.txt"
		failedScans = open(failedScans, 'w')
		for DIR in SetOfScanIDs:
			print DIR
			try:
				args = self.ExtractArgs(DIR)
				self.CallFilter(args, DIR)
			except Exception, e:
				failedScans.write(DIR + '\n')
				print e
				continue
		failedScans.close()


#===============================================================================================
#===============================================================================================
#===============================================================================================


class LaplaceGear:
	"""gear for Pipeline Laplace"""
	def __init__(self):
		self.locs = lo.Locations()


	def ExtractArgs(self, DIR):
		s = DIR + "_LANDMARKS.txt"
		fileobj = open(os.path.join(self.locs.Landmarks, 'Files', s), 'r')

		NoseTip 	= []
		Columella 	= []
		RightAla	= []
		LeftAla		= []

		TrachPoint  	= []
		TrachVectorHead = []
		NosePoint		= []
		NoseVectorHead	= []
		for line in fileobj:
			l = line.split(" ")
			if l[0] == "TracheaCarina":
				TrachPoint.extend( [-float(l[2]),-float(l[3]),float(l[4][:-2])] )
			elif l[0] == "NoseTip":
				NoseTip.extend( [-float(l[2]),-float(l[3]),float(l[4][:-2])] )
			elif l[0] == "Columella":
				Columella.extend( [-float(l[2]),-float(l[3]),float(l[4][:-2])] )
			elif l[0] == "RightAlaRim":
				RightAla.extend( [-float(l[2]),-float(l[3]),float(l[4][:-2])] )
			elif l[0] == "LeftAlaRim":
				LeftAla.extend( [-float(l[2]),-float(l[3]),float(l[4][:-2])] )
			elif l[0] == "NasalSpine":
				NosePoint.extend( [-float(l[2]),-float(l[3]),float(l[4][:-2])] )
			else:
				pass
		NoseTip = np.subtract(NoseTip, Columella)
		LeftAla = np.subtract(LeftAla, RightAla)
		NoseVectorHead = np.cross(NoseTip, LeftAla)
		NoseVectorHead = np.add( NoseVectorHead/np.linalg.norm(NoseVectorHead), NosePoint )
		TrachVectorHead = np.add( TrachPoint, [0, 0, 1] )

		args = []
		args.extend( [NosePoint, list(NoseVectorHead), TrachPoint, list(TrachVectorHead)] )
		print args
		return args


	def CallFilter(self, args, DIR):
		os.chdir(self.locs.AirwayLaplaceSolutionFilter)
		CutFile = DIR + "_CUT.nrrd"
		HeatFile = DIR + "_HEATFLOW.mha"
		CUT_PATH=os.path.join(self.locs.Cut, 'Files', CutFile)
		OUTPUT_PATH=os.path.join(self.locs.Heatflow, 'Files', HeatFile)

		inpu 			= "--input " + CUT_PATH
		output 			= "--output " + OUTPUT_PATH
		nosePoint 		= "--nasalPoint " + str(args[0][0]) + "," + str(args[0][1]) + "," + str(args[0][2])
		noseVectHead 	= "--nasalVectorHead " + str(args[1][0]) + "," + str(args[1][1]) + "," + str(args[1][2])
		trachPoint 		= "--trachealPoint " + str(args[2][0]) + "," + str(args[2][1]) + "," + str(args[2][2])
		trachVectHead 	= "--tracheaVectorHead " + str(args[3][0]) + "," + str(args[3][1]) + "," + str(args[3][2])

		try:
			with time_limit(3600):
				sub.call( ["./AirwayLaplaceSolutionFilter", inpu, output, nosePoint, noseVectHead, trachPoint, trachVectHead])
		except Exception, e:
			print e


	def Filter(self, SetOfScanIDs):
		failedScans = self.locs.Heatflow + "FailedScans.txt"
		failedScans = open(failedScans, 'w')
		for DIR in SetOfScanIDs:
			print DIR
			
			try:
				
				args = self.ExtractArgs(DIR)
				self.CallFilter(args, DIR)
			except Exception, e:
				failedScans.write(DIR + '\n')
				print e
				continue
		failedScans.close()



#===============================================================================================
#===============================================================================================
#===============================================================================================


class CrossGear:
	""" The Gear that generates the cross sections (pipeCrosssections.py)
	"""
	def __init__(self):
		self.locs = lo.Locations()

	def CallFilter(self, D):

		os.chdir(self.locs.ComputeAirwayCrossSections)

		heatfile    = D + "_HEATFLOW.mha"
		modelFile   = D + "_OUTPUT.vtk"
		output 	    = D + "_CROSS.vtp"

		HEAT_PATH	= os.path.join(self.locs.Heatflow, heatfile)
		MODEL_PATH	= os.path.join(self.locs.OutputVtk, modelFile)
		OUTPUT_PATH	= os.path.join(self.locs.Cross, output)

		print "./ComputeAirwayCrossSections", HEAT_PATH, MODEL_PATH, OUTPUT_PATH
		try:
			with time_limit(3600):
				sub.call( ["./ComputeAirwayCrossSections", HEAT_PATH, MODEL_PATH, OUTPUT_PATH])
		except Exception, e:
			print e

	def Filter(self, SetOfScanIDs):
		failedScans = self.locs.Cross + "FailedScans.txt"
		failedScans = open(failedScans, 'w')
		for DIR in SetOfScanIDs:
			try:
				self.callFilter(DIR)
			except Exception, e:
				print e
				failedScans.write(DIR + '\n')
				continue
		failedScans.close()
#===============================================================================================
#===============================================================================================
#===============================================================================================

class ExtractGear:
	"""gear that runs pipeExtractCrosssections"""
	def __init__(self):
		self.locs = lo.Locations()
		self.DATA = []


	def ExtractArgs(self, DIR):
		landmarks = DIR + "_LANDMARKS.txt"
		fileobj = open( os.path.join(self.locs.Landmarks, 'Files', landmarks), 'r' )

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
		fileobj.close()
		return args



	def CallFilter(self, args, D):
		os.chdir(self.locs.ExtractCrossSections)
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
			try:
				sub.call( ["./ExtractCrossSections", tvc, subg, inf, tc, mp, mt, IN_PATH, OUT_PATH])
			except Exception, e:
				print e



	def ExtractALLData(self, DIR):
		""" Puts all of the slice data into a csv file """

		os.chdir(self.locs.GenericDataObjectReader)
		dirVTP = DIR + "_CROSS.vtp"
		vtpFile = os.path.join(self.locs.Cross, 'Files', dirVTP)
		tempFile = self.locs.Slices

		try:
			sub.call( ['./GenericDataObjectReader', vtpFile, tempFile] )
		except Exception, e:
			print e



	def ExtractSpecifiedData(self, DIR):
		""" Extracts the information about fiducials specified in ExtractArgs() """

		DataFName = DIR + '_Data.csv'
		DataFile = open( os.path.join(self.locs.Data, 'Files', DataFName), 'w' )

		os.chdir(self.locs.GenericDataObjectReader)
		dirVTP = DIR + "_ExtractedSegments.vtp"
		vtpFile = os.path.join(self.locs.Segments, 'Files', dirVTP)


		sub.call( ['./GenericDataObjectReader', vtpFile, self.locs.Temp] )
		f = open(self.locs.Temp, 'r')
		
		lines = list(f)
		l = DIR

		for line in lines[:-1]:
			l =  l + ", " + line[:-1]

		if len(lines) > 0:
			DataFile.write(l)

		f.close()




	def GetSliceIndices(self, DIR):
		""" Gets the indices of the TVC and Subglottic slices """

		os.chdir(self.locs.SliceReader)
		dirVTP = DIR + "_ExtractedSegments.vtp"
		vtpFile = os.path.join(self.locs.Segments, 'Files', dirVTP)
		tempFile = 'slice.txt'
		tempFile = os.path.join(self.locs.Segments, tempFile)
		sub.call( ['./SliceReader', vtpFile, tempFile] )

		try:
			f = open(tempFile, 'r')
			lines = list(f)
			sub.call( ['rm', tempFile])
			l = [ int(lines[0][:-1]), int(lines[1][:-1]) , int(lines[2][:-1]) ]
			

			return l

		except Exception, e:
			print e
			return []

	def PlaceMinXASlice(self, inds, DIR):
		""" Adds information about the min XA to the output file """

		DataFName = DIR + '_Data.csv'
		DataFile = open( os.path.join(self.locs.Data, 'Files', DataFName), 'a' )

		SliceFile = open(self.locs.Slices, 'r')

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
		DataFile.write( ', ' + str(ind) + ', ' + l[:-1] )
		com = l.split(',')[2].split(' ')
		com = [float(com[1]), float(com[2]), float(com[3][:-1])]
		return com


	def PlaceMidTracheaSlice(self, inds, DIR):
		""" Adds info about MidTrachea """

		DataFName = DIR + '_Data.csv'
		DataFile = open( os.path.join(self.locs.Data, 'Files', DataFName), 'a' )

		SliceFile = open(self.locs.Slices, 'r')

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
		DataFile.write(l)
		com = l.split(',')[3].split(' ')
		com = [ float(com[1]), float(com[2]), float(com[3][:-1]) ]
		return com




	def Filter(self, SetOfScanIDs):
		failedScans = self.locs.Segments + "FailedScans.txt"
		failedScans = open(failedScans, 'w')
		for DIR in SetOfScanIDs: 

			try:
				args = self.ExtractArgs(DIR)
				self.CallFilter(args, DIR)
				self.ExtractALLData(DIR)
				self.ExtractSpecifiedData(DIR)
				inds = self.GetSliceIndices(DIR)

				if len(inds) > 0:

					args.append( self.PlaceMinXASlice(inds, DIR) )
					args.append( self.PlaceMidTracheaSlice(inds, DIR) )

					self.CallFilter(args, DIR)

			except Exception, e:
				print DIR
				failedScans.write(DIR + '\n')
				print e
				continue
		failedScans.close()


#===============================================================================================
#===============================================================================================
#===============================================================================================


class WriteGear:
	"""gear for Write To CSV"""
	def __init__(self):
		self.locs = lo.Locations()


	def UpdateCSV(self):
		os.chdir( os.path.join( self.locs.Data, 'Files' ) )
		DATA = open(self.locs.AllData, 'w')
		DATA.write("ScanID, TVC_XA, TVC_PER, TVC_COM , Subglottic_XA, Subglottic_PER, Subglottic_COM , InfSub_XA, InfSub_PER, InfSub_COM , MinSliceID, MinSlice_XA, MinSlice_PER, MinSlice_COM, MidT_XA, MidT_PER, MidT_COM \n")
		datafiles = [ f for f in os.listdir('.') if f[0] != '.']
		for f in datafiles:
			fileobj = open(f, 'r')
			for line in list(fileobj):
				DATA.write(line)


	def Filter(self, SetOfScanIDs):
		if len(SetOfScanIDs) > 0:
			self.UpdateCSV()

		
		
		