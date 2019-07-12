

# Possible executions of this script
# ./yadempi script.py #interactive will spawn 3 additional workers
# mpiexec -n 4 ./yadempi script.py #non interactive

NSTEPS=1000 #turn it >0 to see time iterations, else only initilization TODO!HACK
#NSTEPS=50 #turn it >0 to see time iterations, else only initilization
N=50; M=50; #(columns, rows) per thread

import os
from yade import mpy as mp
numThreads = 4


#add spheres
young = 5e6
compFricDegree = 0.0
O.materials.append(FrictMat(young=young, poisson=0.5, frictionAngle = radians(compFricDegree), density= 2600, label='sphereMat'))
O.materials.append(FrictMat(young=young*100, poisson = 0.5, frictionAngle = compFricDegree, density =2600, label='wallMat'))

mn,mx=Vector3(0,0,0),Vector3(90,180,90)
pred = pack.inAlignedBox(mn,mx)
O.bodies.append(pack.regularHexa(pred,radius=2.80,gap=0, material='sphereMat'))


wallIds=aabbWalls([Vector3(-360,-1,-360),Vector3(360,360,360)],thickness=10.0, material='wallMat')
O.bodies.append(wallIds)

collider.verletDist = 1.5
newton.gravity=(0,-10,0) #else nothing would move
tsIdx=O.engines.index(timeStepper) #remove the automatic timestepper. Very important: we don't want subdomains to use many different timesteps...
O.engines=O.engines[0:tsIdx]+O.engines[tsIdx+1:]
O.dt=0.01 #this very small timestep will make it possible to run 2000 iter without merging
#O.dt=0.1*PWaveTimeStep() #very important, we don't want subdomains to use many different timesteps...
newton.gravity=0.05*newton.gravity


#########  RUN  ##########

def collectTiming():
	created = os.path.isfile("collect.dat")
	f=open('collect.dat','a')
	if not created: f.write("numThreads mpi omp Nspheres N M runtime \n")
	from yade import timing
	f.write(str(numThreads)+" "+str(os.getenv('OMPI_COMM_WORLD_SIZE'))+" "+os.getenv('OMP_NUM_THREADS')+" "+str(N*M*(numThreads-1))+" "+str(N)+" "+str(M)+" "+str(timing.runtime())+"\n")
	f.close()

# customize mpy

mp.VERBOSE_OUTPUT=False
mp.DOMAIN_DECOMPOSITION= True
mp.mpirun(NSTEPS,4)
mp.mergeScene() 
if mp.rank == 0: O.save('mergedScene.yade')

#demonstrate getting stuff from workers
if mp.rank==0:
	print("kinetic energy from workers: "+str(mp.sendCommand([1,2],"kineticEnergy()",True)))
