import os
import hashlib
import Gears
import Locations as lo


class Gearbox:
	"""docstring for Gearbox"""
	def __init__(self, GearboxType):
		if GearboxType == 'CUT':
			self.Stage = Gears.CutGear()
		elif GearboxType == 'LAPLACE':
			self.Stage = Gears.LaplaceGear()
		elif GearboxType == 'GENCROSS':
			self.Stage = Gears.CrossGear()
		elif GearboxType == 'EXTRACT':
			self.Stage = Gears.ExtractGear()
		elif GearboxType == 'WRITE':
			self.Stage = Gears.WriteGear()
		else:
			raise Exception("No such gearbox!")


	def Pipe(self, SetOfFiles):
		self.Stage.Filter(SetOfFiles)





		

class InputChecker:
	"""InputChecker identifies the files in the folderList that dont match their sha256"""
	def __init__(self, folderList):
		self.folderList = folderList
		self.Mismatched = set()


	def GetSha256(self, fname):
		f = open(fname, 'r').read()
		m = hashlib.sha256(f)
		return m.hexdigest()


	def GetMismatched(self):
		self.Mismatched.clear()
		for inpu in self.folderList:

			Files  = os.path.join(inpu, 'Files')
			Sha256 = os.path.join(inpu, 'Sha256')

			fs 		= [ f for f in os.listdir(Files) if f[0] != '.']

			for fname in fs:
				dFile = os.path.join(Files, fname)
				sFile = os.path.join(Sha256, fname, '.sha256')
				S256  = self.GetSha256(dFile)

				if os.path.isfile(sFile):
					s = open(sFile, 'r')
					for line in s:
						if line != S256:
							name = fname[:4]
							self.Mismatched.add(name)
							# s = open(sFile, 'w')
							# s.write(S256)
				else:
					name = fname[:4]
					self.Mismatched.add(name)
					# s = open(sFile, 'w')
					# s.write(S256)
		return self.Mismatched








class PipelineStep:
	"""A dynamic class that can be instantiated with any type of
		"Gearbox"  The gearbox is the layer of the pipeline, either
		CUT, LAPLACE, GENCROSS, EXTRACT, or WRITE 
		these are the 5 steps int he pipeline"""
	def __init__(self, InputFolderList, GearboxType):
		self.Checker 	= InputChecker(InputFolderList)
		self.Machine 	= Gearbox(GearboxType)

	def Run(self):
		self.Machine.Pipe( self.Checker.GetMismatched() )
		pass



		


def main():
	loc = lo.Locations()

	list1 = [loc.Cross, loc.Landmarks]
	list2 = [loc.Landmarks, loc.Cross]

	step1 = PipelineStep(list1, 'GENCROSS')
	step2 = PipelineStep(list2, 'EXTRACT')

	step1.Run()




if __name__ == '__main__':
	main()

		