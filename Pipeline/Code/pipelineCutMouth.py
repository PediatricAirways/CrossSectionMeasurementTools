import numpy as np
import os
import subprocess as sub 


def extractArgs(fileobj):
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
	# print args[0][0]
	# print args[1][0]
	return args


# ./AirwayLaplaceSolutionFilter --input /Users/jonathankylstra/heat/1159_Segmentation/1159_Segmented.nrrd --output out.nrrd --nasalPoint 3.57394,-238.325,-104.707 --nasalVectorHead 3.57394,-213.187,-96.0497 --trachealPoint -6.58714,-125.39,-267.102 --tracheaVectorHead -6.43388,-126.136,-246.56
			


def callFilter(args, R, D):
	os.chdir("/Users/jonathankylstra/ComputeAirwayCrossSections/ExcludeMouth/CLI-Plugin/Build/lib/Slicer-4.4/cli-modules")
	first  	= D + "_OUTPUT.nrrd"
	segfile = D + "_CUT.nrrd"
	FIRST_PATH=os.path.join(R,D,first)
	CUT_PATH=os.path.join(R,D,segfile)

	centers  = args[0]
	radii	 = args[1]
	input_paths = []

	for i,x in enumerate(args[0]):
		if i == 0:
			input_paths.append(str(FIRST_PATH))
		else:
			input_paths.append( str(CUT_PATH) )
		
		inpu = "--input " + input_paths[i]
		output   = "--output " + CUT_PATH
		Center   = "--Center " + str(centers[i][0]) + "," + str(centers[i][1]) + "," + str(centers[i][2])
		Radius	 = "--Radius " + str(radii[i])
		call = "./AirwayLaplaceSolutionFilter" + ' ' + inpu + ' ' + output + ' ' + Center + ' ' + Radius
		# print call
		try:
			sub.call( ["./ExcludeSphere", inpu, output, Center, Radius])
		except Exception, e:
			print e




ROOT='/Users/jonathankylstra/DATA'
for DIR in ['1092', '1090', '1100', '1052', '1103', '1108', '1057', '1041']: #[2025]:#[2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2017, 2019, 2021, 2022, 2025]:#[1073, 1059, 1046, 1078, 1108, 1049, 1101, 1042, 1047, 1065]:
	print DIR
	s = DIR + "_CLIPPINGS.txt"
	try:
		f = open(os.path.join(ROOT, DIR, s), 'r')
		args = extractArgs(f)
		callFilter(args, ROOT, DIR)
	except Exception, e:
		print e
		continue



