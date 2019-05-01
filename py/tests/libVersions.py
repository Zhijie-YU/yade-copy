import unittest
import yade.libVersions

class TestLibVersions(unittest.TestCase):
	def setUp(self):
		pass # no setup needed for tests here
	def testLibVersions(self):
		# calling them is enough, to make sure they do not throw any errors.
		cmakeVer = yade.libVersions.getAllVersionsCmake()
		cppVer   = yade.libVersions.getAllVersionsCpp()
		print("cmake versions: ", cmakeVer)
		print("C++ versions: ", cppVer)
		print("\n")
		yade.libVersions.printAllVersions(True)
		yade.libVersions.printAllVersions(False)
		for key,val in cmakeVer.items():
			if((key in cppVer) and (len(val)==2) and (len(cppVer[key])==2)):
				print(str(key)+" version reported by by cmake is ",val," and by C++ is ",cppVer[key])
				if(key!='mpi'):
					self.assertEqual(val[0],cppVer[key][0])
				elif (val[0]!=cppVer[key][0]):
					print('\033[91m'+" Warning: mpi versions are different. Can you help with file py/libVersions.py.in?"+'\033[0m')
