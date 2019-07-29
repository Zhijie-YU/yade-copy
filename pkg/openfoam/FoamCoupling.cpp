// *  Deepak kn : deepak.kunhappan@3sr-grenoble.fr; deepak.kn1990@gmail.com

#ifdef YADE_MPI
#include <mpi.h>
#include "FoamCoupling.hpp"
#include <iostream>

CREATE_LOGGER(FoamCoupling);
YADE_PLUGIN((FoamCoupling));

void FoamCoupling::getRank() {

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &commSize);

}

void FoamCoupling::setNumParticles(int np){

  numParticles = np;
  bodyList.assign(numParticles, -1);
  particleData.assign(numParticles*10, 1e-19); //10 doubles for each sphere;
  procList.assign(numParticles, -1);
  hydroForce.assign(numParticles*6, 1e-19); // 3 force, 3 torque
  castNumParticle(numParticles);
}

void FoamCoupling::setIdList(const boost::python::list& alist) {

  for (unsigned int i=0; i != bodyList.capacity(); ++i){
    bodyList[i] = boost::python::extract<int> (alist[i]);
  }

}


void FoamCoupling::castParticle() {

#pragma omp parallel  for collapse (1)

for (unsigned int i=0; i <  bodyList.capacity(); ++i)
  {
    const Body* b = (*scene -> bodies)[bodyList[i]].get();
    if ( scene-> isPeriodic){
      Vector3r pos = scene->cell->wrapPt( b->state->pos);
      particleData[i*10] = pos[0];
      particleData[i*10+1] = pos[1];
      particleData[i*10+2] = pos[2];
    } else {

    particleData[i*10] = b->state->pos[0];
    particleData[i*10+1] = b->state->pos[1];
    particleData[i*10+2] = b->state->pos[2];
  }
    particleData[i*10+3] = b->state->vel[0];
    particleData[i*10+4] = b->state->vel[1];
    particleData[i*10+5] = b->state->vel[2];
    particleData[i*10+6] = b->state->angVel[0];
    particleData[i*10+7] = b->state->angVel[1];
    particleData[i*10+8] = b->state->angVel[2];
    shared_ptr<Sphere> s = YADE_PTR_DYN_CAST<Sphere>(b->shape);
    particleData[i*10+9] = s->radius;
  }

  MPI_Bcast(&particleData.front(), particleData.capacity(), MPI_DOUBLE, rank, MPI_COMM_WORLD);

}


void FoamCoupling::castNumParticle(int value) {
  MPI_Bcast(&value, 1, MPI_INT, rank, MPI_COMM_WORLD);

}



void FoamCoupling::castTerminate() {
  int value = 10; 
  MPI_Bcast(&value, 1, MPI_INT, rank, MPI_COMM_WORLD);

}



void FoamCoupling::updateProcList()
{
  for (unsigned int i=0; i != bodyList.capacity(); ++i)
  {
     int dummy_val = -5;
     MPI_Allreduce(&dummy_val,&procList[i],1,MPI_INT, MPI_MAX, MPI_COMM_WORLD);
     if (procList[i] < 0 )  std::cout << "Particle not found in FOAM " << std::endl;
   }
}

void FoamCoupling::recvHydroForce() {

  for (unsigned int i=0; i!= procList.capacity(); ++i) {
    int recvFrom = procList[i];
    for (unsigned int j=0; j != 6; ++j) {
     MPI_Recv(&hydroForce[6*i+j],1,MPI_DOUBLE,recvFrom,sendTag,MPI_COMM_WORLD,&status); }
   }
}

void FoamCoupling::setHydroForce() {

  // clear hydroforce before summation

 #pragma omp parallel for collapse(1)
    for (unsigned int i=0; i < bodyList.capacity(); ++i) {
       const Vector3r& fx=Vector3r(hydroForce[6*i], hydroForce[6*i+1], hydroForce[6*i+2]);
       const Vector3r& tx=Vector3r(hydroForce[6*i+3], hydroForce[6*i+4], hydroForce[6*i+5]);
       scene->forces.addForce(bodyList[i], fx);
       scene->forces.addTorque(bodyList[i], tx);
  }

}

void FoamCoupling::sumHydroForce() {
  // clear the vector
  std::fill(hydroForce.begin(), hydroForce.end(), 0.0);
  Real dummy_val = 0.0;
  for (unsigned int i=0; i != bodyList.capacity(); ++i) {
   for (unsigned int j=0; j != 6; ++j){
     MPI_Allreduce(&dummy_val ,&hydroForce[6*i+j],1,MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
   }
  }
}


void FoamCoupling::resetProcList() {
   int capacity = procList.capacity();
   procList.assign(capacity, -1);
}


void FoamCoupling::action() {

  if ( exchangeData()) {
    runCoupling();
    exchangeDeltaT();
  }
  setHydroForce();
}

bool FoamCoupling::exchangeData(){

  return scene->iter%dataExchangeInterval==0;

}

void FoamCoupling::exchangeDeltaT() {


  // Recv foamdt  first and broadcast;
  MPI_Recv(&foamDeltaT,1,MPI_DOUBLE,1,sendTag,MPI_COMM_WORLD,&status);
  //bcast yadedt to others.
  Real  yadeDt = scene-> dt;
  MPI_Bcast(&yadeDt,1,MPI_DOUBLE, rank, MPI_COMM_WORLD);
  // calculate the interval . TODO: to include hydrodynamic time scale if inertial in openfoam
  // here -> hDeltaT = getViscousTimeScale();
  dataExchangeInterval = (long int) ((yadeDt < foamDeltaT) ? foamDeltaT/yadeDt : 1);

}

Real FoamCoupling::getViscousTimeScale() {

//  Real hDeltaT = 0.0;
//  Real dummy = 1e9;
//
//  MPI_Allreduce(&dummy, &hDeltaT, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
//
  return 0;
}

void FoamCoupling::runCoupling() {

    castParticle();
    updateProcList();
    if (isGaussianInterp){
        sumHydroForce();
    } else {
    recvHydroForce();
  }
}

void FoamCoupling::killMPI() { 
  castTerminate(); 
  MPI_Finalize();
}

#endif
