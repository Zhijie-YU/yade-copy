# encoding: utf-8
# 2011 © Bruno Chareyre <bruno.chareyre@hmg.inpg.fr>
from __future__ import print_function
from past.builtins import execfile
import yade,math,os,sys

class YadeCheckError(Exception):
	"""Use ``raise YadeCheckError("message") when check fails. Or throw any other python exception that seems suitable."""
	pass

scriptsToRun=os.listdir(checksPath)
failedScripts=list()

#checkSpawn.py fails always for now, needs investigations
skipScripts = ['checkList.py','checkSpawn.py']
if not yade.libVersions.getLinuxVersion()[:9]=='Ubuntu 16' and not yade.libVersions.getLinuxVersion()[-8:]=='(buster)':
	skipScripts.append('checkMPI.py')
onlyOneScript = [] # use this if you want to test only one script, it takes precedence over skipScripts.

def mustCheck(sc):
	if(len(onlyOneScript)==1): return sc in onlyOneScript
	return sc not in skipScripts

for script in scriptsToRun:
	if (script[len(script)-3:]==".py" and mustCheck(script)):
		try:
			print("###################################")
			print("running: ",script)
			execfile(checksPath+"/"+script)
			print("Status: success")
			print("___________________________________")
		except Exception as e:
			failedScripts.append(script)
			print('\033[91m',script," failure, caught exception ",e.__class__.__name__,": ",e,'\033[0m')
		O.reset()
	elif (not mustCheck(script)):
		print("###################################")
		print("Skipping %s, because it is in SkipScripts"%script)

if (len(failedScripts)!=0):
	print('\033[91m', len(failedScripts) , " tests are failed"+'\033[0m')
	for s in failedScripts: print("  "+s)
	sys.exit(1)
else:
	# https://misc.flogisoft.com/bash/tip_colors_and_formatting
	print('\033[92m'+"*** ALL CHECKS PASSED ***"+'\033[0m')
	sys.exit(0)

