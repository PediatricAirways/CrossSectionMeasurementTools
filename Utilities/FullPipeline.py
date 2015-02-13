import os
import subprocess as sub 
import plumbum


###########
#  THIS IS NOT DONE YET!!!!
###########



def copyNeededFiles( ROOT, DIR ):
	
	r = plumbum.machines.ssh_machine.SshMachine("10.171.2.221", user="schuyler")

	fro = r.path( '/home/cory/Projects/AirwaySegmenter-files/CRL/' + DIR + "/" + DIR + "_OUTPUT.vtk" )
	to = plumbum.local.path(ROOT + DIR + '/' + DIR + '_OUTPUT.vtk' )
	
	fro1 = r.path('/home/cory/Projects/AirwaySegmenter-files/CRL/' + DIR + "/" + DIR + '_OUTPUT.vtp')
	to1 = plumbum.local.path(ROOT + DIR + '/' + DIR + '_OUTPUT.vtp')
	
	fro2 = r.path('/home/cory/Projects/AirwaySegmenter-files/CRL/' + DIR + "/" + DIR + '_LANDMARKS.txt')
	to2 = plumbum.local.path(ROOT + DIR + '/' + DIR + '_LANDMARKS.txt')
	
	try:
		plumbum.path.utils.copy(fro, to)
		plumbum.path.utils.copy(fro1, to1)
		plumbum.path.utils.copy(fro2, to2)
	except Exception, e:
		print e



def ExtractLaplaceArgs( ROOT, DIR ):
	s = DIR + "_LANDMARKS.txt"
	f = open(os.path.join(ROOT, DIR, s), 'r')
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

	f.close()	
	return args



def LaplaceFilter( args ):
	os.chdir("/Users/jonathankylstra/heatflowplugin/Build/lib/Slicer-4.4/cli-modules")
	segfile = D + "_OUTPUT.nrrd"
	heatfile = D + "_HEATFLOW.mhd"
	SEGMENTATION_PATH=os.path.join(R,D,segfile)
	OUTPUT_PATH=os.path.join(R,D,heatfile)

	inpu = "--input " + SEGMENTATION_PATH
	output = "--output " + OUTPUT_PATH
	nosePoint = "--nasalPoint " + str(args[0][0]) + "," + str(args[0][1]) + "," + str(args[0][2])
	noseVectHead = "--nasalVectorHead " + str(args[1][0]) + "," + str(args[1][1]) + "," + str(args[1][2])
	trachPoint = "--trachealPoint " + str(args[2][0]) + "," + str(args[2][1]) + "," + str(args[2][2])
	trachVectHead = "--tracheaVectorHead " + str(args[3][0]) + "," + str(args[3][1]) + "," + str(args[3][2])

	sub.call( ["./AirwayLaplaceSolutionFilter", inpu, output, nosePoint, noseVectHead, trachPoint, trachVectHead])



def CrossFilter( ROOT, DIR ):
	os.chdir("/Users/jonathankylstra/Cross-bin/lib/Slicer-4.4/cli-modules")
	heatfile    = DIR + "_HEATFLOW.mhd"
	modelFile   = DIR + "_OUTPUT.vtk"
	output 	    = DIR + "_CROSS.vtp"
	HEAT_PATH	= os.path.join( ROOT, DIR, heatfile )
	MODEL_PATH	= os.path.join( ROOT, DIR, modelFile )
	OUTPUT_PATH	= os.path.join( ROOT, DIR, output )

	sub.call( ["./ComputeAirwayCrossSections", HEAT_PATH, MODEL_PATH, OUTPUT_PATH] )



def ExtractSectionArgs( ROOT, DIR ):
	s = DIR + "_LANDMARKS.txt"
	f = open(os.path.join(ROOT, DIR, s), 'r')

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



def SectionFilter( args, ROOT, DIR ):
	os.chdir("/Users/jonathankylstra/Cross-bin/lib/Slicer-4.4/cli-modules")
	infile = DIR + "_CROSS.vtp"
	outfile = DIR + "_ExtractedSegments.vtp"
	IN_PATH=os.path.join( ROOT, DIR, infile )
	OUT_PATH=os.path.join( ROOT, DIR, outfile )

	tvc = "--queryPoints " + str( args[0][0] ) + "," + str( args[0][1] ) + "," + str( args[0][2] )
	subg = "--queryPoints " + str( args[1][0] ) + "," + str( args[1][1] ) + "," + str( args[1][2] )
	inf = "--queryPoints " + str( args[2][0] ) + "," + str( args[2][1] ) + "," + str( args[2][2] )

	sub.call( ["./ExtractCrossSections", tvc, subg, inf, IN_PATH, OUT_PATH])



def extendLandmarks( ROOT, DIR ):
	path = '/Users/jonathankylstra/MyFiles/Kitware/UNCPediatricAirway/ScanFiducials/' + DIR + '_InferiorSubglottis.fcsv'
	x = open( path , 'r' )
	lines = list(x)
	line = lines[3].split(',')
	xyz = line[1] + ' ' + line[2] + ' ' + line[3]
	newline = xyz + '\n'
	appendText = "InferiorSubglottis : " + newline
	x.close()
	f = open( os.path.join(ROOT, DIR, DIR + "_LANDMARKS.txt"), 'a')
	f.write(appendText)
	f.close()



def extractData(ROOT, DIR, outfile):
	os.chdir("/Users/jonathankylstra/DATA/Generic-Build/GenericDataObjectReader.app/Contents/MacOS/")
	dirVTP = DIR + "_ExtractedSegments.vtp"
	vtpFile = os.path.join(ROOT, DIR, dirVTP)
	tempFile = '/Users/jonathankylstra/DATA/temp.csv'


	sub.call( ['./GenericDataObjectReader', vtpFile, tempFile] )
	f = open(tempFile, 'r')
	
	lines = list(f)
	fidID = ["TVC", "Subglottic", "InferiorSubglottis", "Min XA"]

	for i, line in enumerate(lines):
		l =  DIR + ", " + fidID[i] + ", " + line
		outfile.write(l)

	f.close()


def main():
	ROOT='/Users/jonathankylstra/DATA/'
	dirs = ['1019','1092','1031','1048','1061','1005','1079','1052','1094','1086','1073','1059','1043','1089','1046','1062','1098','1035','1078','1102','1010','1036','1041','1032','1063','1097','1058','1108','1067','1095','1069','1044','1090','1045','1053','1049','1057','1077','1088','1101','1004','1085','1103','1042','1047','1065','1071','1100']

	d = "data.csv"
	x = open(os.path.join(ROOT, d), 'w')
	x.write("ScanID, FiducialID, COM_R, COM_A, COM_S, AREA, PERIMETER\n")

	for DIR in dirs:

		print DIR

		try:
			#copy the files
			copyNeededFiles( ROOT, DIR )

			#run Laplace
			args = ExtractLaplaceArgs( ROOT, DIR )
			LaplaceFilter( args )

			#run Cross Section
			CrossFilter( ROOT, DIR )

			#run Section Extraction
			extendLandmarks( ROOT, DIR )
			args = ExtractSectionArgs( ROOT, DIR )
			SectionFilter( args, ROOT, DIR )

			#extract the data
			ExtractData( ROOT, DIR, x)

		except Exception, e:
			print e
			continue

	sub.call(["rm", '/Users/jonathankylstra/DATA/temp.csv'])


if __name__ == '__main__':
	main()
