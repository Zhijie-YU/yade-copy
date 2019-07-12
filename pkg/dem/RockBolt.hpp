/*CWBoon 2016 */
/* Please cite: */
/* CW Boon, GT Houlsby, S Utili (2015).  Designing Tunnel Support in Jointed Rock Masses Via the DEM.  Rock Mechanics and Rock Engineering,  48 (2), 603-632. */
#ifdef YADE_POTENTIAL_BLOCKS
#pragma once
#include<pkg/dem/PotentialBlock.hpp>
#include<pkg/dem/PotentialBlock2AABB.hpp>

#include <vector>
#include <pkg/common/PeriodicEngines.hpp>

#include <stdio.h>

#include <vtkPolyData.h>

// https://codeyarns.com/2014/03/11/how-to-selectively-ignore-a-gcc-warning/
// https://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcomment"
// Code that generates this warning, Note: we cannot do this trick in yade. If we have a warning in yade, we have to fix it! See also https://gitlab.com/yade-dev/trunk/merge_requests/73
// This method will work once g++ bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53431#c34 is fixed.
#include<vtkTriangle.h>
#pragma GCC diagnostic pop

#include <vtkSmartPointer.h>
#include <vtkFloatArray.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkStructuredPoints.h>
#include <vtkStructuredPointsWriter.h>
#include <vtkWriter.h>
#include <vtkExtractVOI.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkXMLStructuredGridWriter.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkAppendPolyData.h>
#include <vtkCylinderSource.h>
#include <ClpSimplex.hpp>
#include <CoinHelperFunctions.hpp>
#include <CoinTime.hpp>
#include <CoinBuild.hpp>
#include <CoinModel.hpp>

class RockBolt: public PeriodicEngine{	

  	public:
		#if 0
		struct Bolts{
			Bolts(Vector3r pt1, Vector3r  pt2){startingPoint = pt1; endPoint=pt2; }
			Vector3r startingPoint;
			Vector3r endPoint;
			
			/* variables stored in sequence starting from the block closest to the opening */
			vector<int> blockIDs; /*blocks intersected */
			vector<Vector3r> localCoordinates; /*local coordinates inside blocks */
			vector<double> initialLength;
		};
		vector<Bolts> bolt;
		#endif

		Vector3r getNodeDistance(const PotentialBlock* cm1,const State* state1,const PotentialBlock* cm2,const State* state2, const Vector3r localPt1, const Vector3r localPt2);
		bool installBolts(const PotentialBlock* cm1,const State* state1,const Vector3r startingPt,const Vector3r direction, const double length, Vector3r& intersectionPt);
		double evaluateFNoSphereVol(const PotentialBlock* s1,const State* state1, const Vector3r newTrial);
		bool intersectPlane(const PotentialBlock* s1,const State* state1,const Vector3r startingPt,const Vector3r direction, const double length, Vector3r& intersectionPt, const Vector3r plane, const double planeD);
  		virtual void action(void);
  	YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(RockBolt,PeriodicEngine,"Engine recording potential blocks as surfaces into files with given periodicity.",
		((double,normalStiffness,0.0 ,,"EA/L"))
		((double,shearStiffness,0.0 ,,"stiffness"))
		((double,axialStiffness,0.0 ,,"EA"))
		((bool,useMidPoint,false ,,"large length"))
		((double,halfActiveLength,0.02 ,,"stiffness"))
		((bool,resetLengthInit,false ,,"reset length for pretension"))
		((Vector3r,startingPoint,Vector3r(0,0,0) ,,"startingPt"))
		((double,boltLength,0.0 ,,"startingPt"))
		((Vector3r,boltDirection,Vector3r(0,0,0) ,,"direction"))
		((vector<int>,blockIDs, ,,"ids"))
		((double,displacements, ,,"ids"))
		((vector<Vector3r>,localCoordinates, ,,"local coordinates of intersection"))
		((vector<double>,initialLength, ,,"initial length"))	
		((vector<Vector3r>,initialDirection, ,,"initial length"))
		((vector<Vector3r>,nodeDistanceVec, ,,"nodeDistance"))
		((vector<Vector3r>,nodePosition, ,,"nodePosition"))
		((vector<double>,distanceFrCentre, ,,"nodePosition"))
		((vector<double>,forces, ,,"force"))
		((vector<double>,axialForces, ,,"force"))
		((vector<double>,shearForces, ,,"force"))
		((double,openingRad,5.0 ,,"estimated opening radius"))
		((double,preTension,0.0 ,,"prestressed tension"))
		((double,averageForce,0.0 ,,"averageForce"))
		((double,maxForce,0.0 ,,"maxForce"))
		((bool,installed,false ,,"installed?"))
		((bool,openingCreated,false ,,"opening created?"))
		((vector<bool>,ruptured, ,,"ruptured"))
		((Real,axialMax,1000000000 ,,"maximum axial force"))
		((Real,shearMax,1000000000 ,,"maximum shear force"))
		((int,vtkIteratorInterval,10000 ,,"how often to print vtk"))	
		((int,vtkRefTimeStep,1 ,,"first timestep to print vtk"))	
		((string,fileName,,,"File prefix to save to"))	
		((string,name,,,"File prefix to save to"))
		,
			
		,
  	);


};
REGISTER_SERIALIZABLE(RockBolt);

#endif // YADE_POTENTIAL_BLOCKS
