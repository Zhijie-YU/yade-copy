# encoding: utf-8
# 2011 © Bruno Chareyre <bruno.chareyre@hmg.inpg.fr>
from __future__ import print_function
from past.builtins import execfile
import yade,math,os,sys

scriptsToRun=os.listdir(checksPath)
resultStatus = 0
nFailed=0
failedScripts=list()

skipScripts = ['checkList.py','checkPolyhedraCrush.py']

for script in scriptsToRun:
	if (script[len(script)-3:]==".py" and script not in skipScripts):
		try:
			print("###################################")
			print("running: ",script)
			execfile(checksPath+"/"+script)
			if (resultStatus>nFailed):
				print("Status: FAILURE!!!")
				nFailed=resultStatus
				failedScripts.append(script)
			else:
				print("Status: success")
			print("___________________________________")
		except Exception as e:
			resultStatus+=1
			nFailed=resultStatus
			failedScripts.append(script)
			print(script," failure, caught exception: ",e)
		O.reset()
	elif (script in skipScripts):
		print("###################################")
		print("Skipping %s, because it is in SkipScripts"%script)
		
if (resultStatus>0):
	print(resultStatus, " tests are failed")
	for s in failedScripts: print("  "+s)
	sys.exit(1)
else:
	sys.exit(0)

