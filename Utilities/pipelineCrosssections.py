import os
import subprocess as sub 

			


def callFilter(R, D):
	os.chdir("/Users/jonathankylstra/Cross-bin/lib/Slicer-4.4/cli-modules")
	heatfile    = D + "_HEATFLOW.mhd"
	modelFile   = D + "_OUTPUT.vtk"
	output 	    = D + "_CROSS.vtp"
	HEAT_PATH	= os.path.join(R,D,heatfile)
	MODEL_PATH	= os.path.join(R,D,modelFile)
	OUTPUT_PATH	= os.path.join(R,D,output)
	print "./ComputeAirwayCrossSections", HEAT_PATH, MODEL_PATH, OUTPUT_PATH
	try:
		sub.call( ["./ComputeAirwayCrossSections", HEAT_PATH, MODEL_PATH, OUTPUT_PATH])
	except Exception, e:
		print e




ROOT='/Users/jonathankylstra/DATA'

dirs = ['1019','1092','1031','1048','1061','1005','1079','1052','1094','1086','1073','1059','1043','1089','1046','1062','1098','1035','1078','1102','1010','1036','1041','1032','1063','1097','1058','1108','1067','1095','1069','1044','1090','1045','1053','1049','1057','1077','1088','1101','1004','1085','1103','1042','1047','1065','1071','1100']
d = ['1103', '1108', '1057', '1041', '1090','1092', '1100', '1052', ]

for DIR in d: #onlyfolders:
	# print DIR
	try:
		callFilter(ROOT, DIR)
	except Exception, e:
		print e
		continue



