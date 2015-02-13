import numpy as np
import os
import subprocess as sub 


def extractArgs(fileobj):
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
	# print NoseTip
	LeftAla = np.subtract(LeftAla, RightAla)
	# print LeftAla
	NoseVectorHead = np.cross(NoseTip, LeftAla)
	# print NoseVectorHead
	NoseVectorHead = np.add( NoseVectorHead/np.linalg.norm(NoseVectorHead), NosePoint )
	# print NoseVectorHead
	TrachVectorHead = np.add( TrachPoint, [0, 0, 1] )
	args = []
	args.extend( [NosePoint, list(NoseVectorHead), TrachPoint, list(TrachVectorHead)] )
	# print args
	return args


# ./AirwayLaplaceSolutionFilter --input /Users/jonathankylstra/heat/1159_Segmentation/1159_Segmented.nrrd --output out.nrrd --nasalPoint 3.57394,-238.325,-104.707 --nasalVectorHead 3.57394,-213.187,-96.0497 --trachealPoint -6.58714,-125.39,-267.102 --tracheaVectorHead -6.43388,-126.136,-246.56
			


def callFilter(args, R, D):
	os.chdir("/Users/jonathankylstra/heatflowplugin/Build/lib/Slicer-4.4/cli-modules")
	segfile = D + "_CUT.nrrd"
	heatfile = D + "_HEATFLOW.mhd"
	SEGMENTATION_PATH=os.path.join(R,D,segfile)
	OUTPUT_PATH=os.path.join(R,D,heatfile)

	inpu 			= "--input " + SEGMENTATION_PATH
	output 			= "--output " + OUTPUT_PATH
	nosePoint 		= "--nasalPoint " + str(args[0][0]) + "," + str(args[0][1]) + "," + str(args[0][2])
	noseVectHead 	= "--nasalVectorHead " + str(args[1][0]) + "," + str(args[1][1]) + "," + str(args[1][2])
	trachPoint 		= "--trachealPoint " + str(args[2][0]) + "," + str(args[2][1]) + "," + str(args[2][2])
	trachVectHead 	= "--tracheaVectorHead " + str(args[3][0]) + "," + str(args[3][1]) + "," + str(args[3][2])

	stdIn = inpu + output + nosePoint + noseVectHead + trachPoint + trachVectHead

	try:
		sub.call( ["./AirwayLaplaceSolutionFilter", inpu, output, nosePoint, noseVectHead, trachPoint, trachVectHead])
	except Exception, e:
		print e




ROOT='/Users/jonathankylstra/DATA'
onlyfolders = [ f for f in os.listdir(ROOT) if not os.path.isfile(os.path.join(ROOT,f)) ]
for DIR in [ '1103', '1108', '1057', '1041', '1090', '1092', '1100', '1052']:#,['1019','1092','1031','1048','1061','1005','1079','1052','1094','1086','1073','1059','1043','1089','1046','1062','1098','1035','1078','1102','1010','1036','1041','1032','1063','1097','1058','1108','1067','1095','1069','1044','1090','1045','1053','1049','1057','1077','1088','1101','1004','1085','1103','1042','1047','1065','1071','1100']:#[2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2017, 2019, 2021, 2022, 2025]:#[1073, 1059, 1046, 1078, 1108, 1049, 1101, 1042, 1047, 1065]:
	print DIR
	s = DIR + "_LANDMARKS.txt"
	try:
		f = open(os.path.join(ROOT, DIR, s), 'r')
		args = extractArgs(f)
		callFilter(args, ROOT, DIR)
	except Exception, e:
		print e
		continue



