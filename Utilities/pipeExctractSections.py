import os
import subprocess as sub 
import numpy as np


def extractArgs(fileobj):
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
	return args

 
			


def callFilter(args, R, D):
	os.chdir("/Users/jonathankylstra/Cross-bin/lib/Slicer-4.4/cli-modules")
	infile = D + "_CROSS.vtp"
	outfile = D + "_ExtractedSegments.vtp"
	IN_PATH=os.path.join(R,D,infile)
	OUT_PATH=os.path.join(R,D,outfile)

	tvc = "--queryPoints " + str(args[0][0]) + "," + str(args[0][1]) + "," + str(args[0][2])
	subg = "--queryPoints " + str(args[1][0]) + "," + str(args[1][1]) + "," + str(args[1][2])
	inf = "--queryPoints " + str(args[2][0]) + "," + str(args[2][1]) + "," + str(args[2][2])
	tc = "--queryPoints " + str(args[3][0]) + "," + str(args[3][1]) + "," + str(args[3][2])


	try:
		sub.call( ["./ExtractCrossSections", tvc, subg, inf, tc, IN_PATH, OUT_PATH])
	except Exception, e:
		print e



def extendLandmarks(num, f):
	path = '/Users/jonathankylstra/MyFiles/Kitware/UNCPediatricAirway/ScanFiducials/' + num + '_InferiorSubglottis.fcsv'
	x = open( os.path.join( path ) )
	lines = list(x)
	line = lines[3].split(',')
	xyz = line[1] + ' ' + line[2] + ' ' + line[3]
	newline = xyz + '\n'
	appendText = "InferiorSubglottis : " + newline
	x.close()
	f.write(appendText)




def extractData(ROOT, DIR, outfile):
	""" Extracts the information about fiducials specified in extractArgs() """

	os.chdir("/Users/jonathankylstra/DATA/Generic-Build/GenericDataObjectReader.app/Contents/MacOS/")
	dirVTP = DIR + "_ExtractedSegments.vtp"
	vtpFile = os.path.join(ROOT, DIR, dirVTP)
	tempFile = '/Users/jonathankylstra/DATA/temp.csv'


	sub.call( ['./GenericDataObjectReader', vtpFile, tempFile] )
	f = open(tempFile, 'r')
	
	lines = list(f)
	l = DIR
	# outfile.write(DIR)

	for line in lines[:-1]:
		l =  l + ", " + line[:-1]

	if len(lines) > 0:
		outfile.write(l)

	
	f.close()



def extractALLData(ROOT, DIR):
	""" Puts all of the slice data into a csv file """

	os.chdir("/Users/jonathankylstra/DATA/Generic-Build/GenericDataObjectReader.app/Contents/MacOS/")
	dirVTP = DIR + "_CROSS.vtp"
	vtpFile = os.path.join(ROOT, DIR, dirVTP)
	tempFile = '/Users/jonathankylstra/DATA/' + DIR + '/slices.csv'

	try:
		sub.call( ['./GenericDataObjectReader', vtpFile, tempFile] )
	except Exception, e:
		print e
	



def GetSliceIndices(ROOT, DIR):
	""" Gets the indices of the TVC and Subglottic slices """

	os.chdir("/Users/jonathankylstra/DATA/Slices-Build/SliceReader.app/Contents/MacOS/")
	dirVTP = DIR + "_ExtractedSegments.vtp"
	vtpFile = os.path.join(ROOT, DIR, dirVTP)
	tempFile = '/Users/jonathankylstra/DATA/' + DIR + '/slice.txt'
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



def PlaceMinXASlice(inds, ROOT, DIR, outfile):
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
	

def PlaceMidTracheaSlice(inds, ROOT, DIR, outfile):
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





ROOT='/Users/jonathankylstra/DATA'
dirs = ['1019','1092','1031','1048','1061','1005','1079','1052','1094','1086','1073','1059','1043','1089','1046','1062','1098','1035','1078','1102','1010','1036','1041','1032','1063','1097','1058','1108','1067','1095','1069','1044','1090','1045','1053','1049','1057','1077','1088','1101','1004','1085','1103','1042','1047','1065','1071','1100']
d = "data.csv"
Y = ['1061']
x = open(os.path.join(ROOT, d), 'w')
x.write("ScanID, TVC_XA, TVC_PER, TVC_COM , Subglottic_XA, Subglottic_PER, Subglottic_COM , InfSub_XA, InfSub_PER, InfSub_COM , MinSliceID, MinSlice_XA, MinSlice_PER, MinSlice_COM, MidT_XA, MidT_PER, MidT_COM \n")
for DIR in dirs: 
	# print DIR
	s = DIR + "_LANDMARKS.txt"
	try:
		
		
		# f = open(os.path.join(ROOT, DIR, s), 'a')
		# extendLandmarks(DIR, f)
		# f.close()
		# f = open( os.path.join(ROOT, DIR, s), 'r' )
		# args = extractArgs(f)
		# callFilter(args, ROOT, DIR)
		# extractALLData(ROOT, DIR)
		extractData(ROOT, DIR, x)
		inds = GetSliceIndices(ROOT, DIR)
		if len(inds) > 0:
			print DIR
			PlaceMinXASlice(inds, ROOT, DIR, x)
			PlaceMidTracheaSlice(inds, ROOT, DIR, x)


		

	except Exception, e:
		print e
		continue

sub.call(["rm", '/Users/jonathankylstra/DATA/temp.csv'])



