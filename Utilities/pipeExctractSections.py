import numpy as np
import os
import subprocess as sub 


def extractArgs(fileobj):
	Subglottic 	= []
	InfSubglot 	= []
	TVC 		= []


	for line in fileobj:
		l = line.split(" ")
		if l[0] == "InferiorSubglottis":
			InfSubglot.extend( [float(l[2]),float(l[3]),float(l[4][:-2])] )
		elif l[0] == "Subglottic":
			Subglottic.extend( [float(l[2]),float(l[3]),float(l[4][:-2])] )
		elif l[0] == "TVC":
			TVC.extend( [float(l[2]),float(l[3]),float(l[4][:-2])] )
		else:
			pass

	args = []
	args.extend( [TVC, Subglottic, InfSubglot] )
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

	stdIn = "./ExtractCrossSections" + " " + tvc + " " + subg + " " + inf + " " + IN_PATH + " " + OUT_PATH
	print stdIn

	try:
		sub.call( ["./ExtractCrossSections", tvc, subg, inf, IN_PATH, OUT_PATH])
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
	os.chdir("/Users/jonathankylstra/DATA/Generic-Build/SliceToCsv.app/Contents/MacOS/")
	dirVTP = DIR + "_ExtractedSegments.vtp"
	vtpFile = os.path.join(ROOT, DIR, dirVTP)
	tempFile = '/Users/jonathankylstra/DATA/temp.csv'


	sub.call( ['./SliceToCsv', vtpFile, tempFile] )
	f = open(tempFile, 'r')
	
	lines = list(f)
	fidID = ["TVC", "Subglottic", "InferiorSubglottis", "Min XA"]

	for i, line in enumerate(lines):
		l =  DIR + ", " + fidID[i] + ", " + line
		outfile.write(l)

	f.close()





ROOT='/Users/jonathankylstra/DATA'
# onlyfolders = [ f for f in os.listdir(ROOT) if not os.path.isfile(os.path.join(ROOT,f)) ]
dirs = ['1073', '1059', '1046', '1078', '1108', '1049', '1101', '1042', '1047', '1065']
d = "data.csv"
x = open(os.path.join(ROOT, d), 'w')
x.write("ScanID, FiducialID, COM_R, COM_A, COM_S, AREA, PERIMETER\n")
for DIR in dirs: 
	s = DIR + "_LANDMARKS.txt"
	try:
		
		
		# f = open(os.path.join(ROOT, DIR, s), 'a')
		# extendLandmarks(DIR, f)
		# f.close()
		# f = open( os.path.join(ROOT, DIR, s), 'r' )
		# args = extractArgs(f)
		# callFilter(args, ROOT, DIR)
		extractData(ROOT, DIR, x)
	except Exception, e:
		print e
		continue

sub.call(["rm", '/Users/jonathankylstra/DATA/temp.csv'])



