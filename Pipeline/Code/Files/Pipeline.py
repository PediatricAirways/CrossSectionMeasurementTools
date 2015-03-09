import os
import hashlib
import Gears
import Locations as lo
import subprocess as sub

		

class InputChecker:
	"""InputChecker identifies the files in the FolderList that dont match their sha256"""
	def __init__(self, FolderList, Executables):
		self.FolderList  = FolderList
		self.Executables = Executables 
		self.Mismatched  = set()


	def GetSha256(self, fname):
		f = open(fname, 'r').read()
		m = hashlib.sha256(f)
		return m.hexdigest()



	def CheckExecutables(self):
		loc = lo.Locations()
		for e in self.Executables:
			eFile = getattr(loc, e) + e
			sFile = e + '.Sha256'
			sFile = os.path.join(loc.CodeSha256, sFile)
			S256  = self.GetSha256(eFile)

			if os.path.isfile(sFile):
				s = open(sFile, 'r')
				for line in s:
					if line != S256:
						print S256
						s = open(sFile, 'w')
						s.write(S256)
						return True
				continue

			s = open(sFile, 'w')
			print S256
			s.write(S256)
			return True

		return False



	def GetMismatched(self):
		print "Generating Update Set"
		self.Mismatched.clear()

		ReRun 	= self.CheckExecutables()

		for inpu in self.FolderList:

			Files  	= os.path.join(inpu, 'Files')
			Sha256 	= os.path.join(inpu, 'Sha256')

			fs 		= [ f for f in os.listdir(Files) if f[0] != '.']

			for fname in fs:

				dFile = os.path.join(Files, fname)
				sFile = fname + '.sha256'
				sFile = os.path.join(Sha256, sFile)
				S256  = self.GetSha256(dFile)

				if os.path.isfile(sFile):
					s = open(sFile, 'r')
					for line in s:
						if (line != S256) or ReRun:
							name = fname[:4]
							self.Mismatched.add(name)
							s = open(sFile, 'w')
							s.write(S256)
							print name
					continue

				name = fname[:4]
				self.Mismatched.add(name)
				s = open(sFile, 'w')
				s.write(S256)
				print name

		print  self.Mismatched
		return self.Mismatched









class PipelineStep:
	"""A dynamic class that can be instantiated with any type of
		"Gearbox"  The gearbox is the layer of the pipeline, either
		CUT, LAPLACE, GENCROSS, EXTRACT, or WRITE 
		these are the 5 steps int he pipeline"""
	def __init__(self, InputFolderList, Executables, Gear):
		self.Checker 	= InputChecker(InputFolderList, Executables)
		self.Gearbox 	= Gear

	def Run(self):
		self.Gearbox.Filter( self.Checker.GetMismatched() )
		pass



		


def main():
	loc = lo.Locations()

	InputList1 = [loc.OutputNrrd, loc.Clippings]
	InputList2 = [loc.Landmarks, loc.Cut]
	InputList3 = [loc.Heatflow, loc.OutputVtk]
	InputList4 = [loc.Landmarks, loc.Cross]
	InputList5 = [loc.Data]

	ExecutableList1 = ['ExcludeSphere']
	ExecutableList2 = []
	ExecutableList3 = []
	ExecutableList4 = []
	ExecutableList5 = []


	step1 = PipelineStep(InputList1, ExecutableList1, Gears.CutGear())
	step1.Run()

	# step2 = PipelineStep(InputList2, Gears.LaplaceGear())
	# step2.Run()

	# step3 = PipelineStep(InputList3, Gears.CrossGear())
	# step3.Run()

	# step4 = PipelineStep(InputList4, Gears.ExtractGear())
	# step4.Run()

	# step5 =PipelineStep(InputList5, Gears.WriteGear())
	# step5.Run()

	# sub.call(['rm', loc.Slices])
	# sub.call(['rm', loc.Temp])
	




if __name__ == '__main__':
	main()

		