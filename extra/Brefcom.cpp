// 2008 © Václav Šmilauer <eudoxos@arcig.cz> 
#include"Brefcom.hpp"
#include<yade/core/MetaBody.hpp>
#include<yade/pkg-dem/BodyMacroParameters.hpp>


YADE_PLUGIN("BrefcomMakeContact","BrefcomContact","BrefcomLaw");

/********************** BrefcomMakeContact ****************************/
CREATE_LOGGER(BrefcomMakeContact);


//! @todo Formulas in the following should be verified
void BrefcomMakeContact::go(const shared_ptr<PhysicalParameters>& pp1, const shared_ptr<PhysicalParameters>& pp2, const shared_ptr<Interaction>& interaction){

	const shared_ptr<SpheresContactGeometry>& contGeom=dynamic_pointer_cast<SpheresContactGeometry>(interaction->interactionGeometry);
	assert(contGeom); // for now, don't handle anything other than SpheresContactGeometry

	if(!interaction->isNew && interaction->interactionPhysics){
		const shared_ptr<BrefcomContact> contPhys=dynamic_pointer_cast<BrefcomContact>(interaction->interactionPhysics);
		assert(contPhys);
		contPhys->prevNormal=contGeom->normal;
	} else {
		interaction->isNew; // just in case
		//TRACE;

		const shared_ptr<BodyMacroParameters>& elast1=static_pointer_cast<BodyMacroParameters>(pp1);
		const shared_ptr<BodyMacroParameters>& elast2=static_pointer_cast<BodyMacroParameters>(pp2);

		shared_ptr<BrefcomContact> contPhys(new BrefcomContact());
		interaction->interactionPhysics=contPhys;


		Real E12=2*elast1->young*elast2->young/(elast1->young+elast2->young); // harmonic Young's modulus average
		Real nu12=2*elast1->poisson*elast2->poisson/(elast1->poisson+elast2->poisson); // dtto for Poisson ratio 
		Real S12=Mathr::PI*pow(min(contGeom->radius1,contGeom->radius2),2); // "surface" of interaction

		contPhys->omegaPl=0;
		/* let's try initial distance for d0; that should prevent packings with inner stresses from exploding */
		//contPhys->d0=contGeom->radius1 + contGeom->radius2; // equilibrium distace is "just touching"
		contPhys->d0=(elast1->se3.position-elast2->se3.position).Length(); // equilibrium distance is now
		contPhys->Kn=(E12*S12/contPhys->d0)*( (1+alpha)/(beta*(1+nu12) + gamma*(1-alpha*nu12)));
		contPhys->Ks=contPhys->Kn*(1-alpha*nu12)/(1+nu12);
		contPhys->zeta=zeta; // TODO: calculate zeta from Gf
		contPhys->frictionAngle=.5*(elast1->frictionAngle+elast2->frictionAngle);
		contPhys->FnMax=S12*sigmaT;
		contPhys->cohesion=S12*sigmaC;
		contPhys->prevNormal=contGeom->normal;
		contPhys->Fs=Vector3r::ZERO;

		// HACK: after beginning all links will be without cohesion (omegaPl=1) !!!
		if(Omega::instance().getCurrentIteration()>50) contPhys->omegaPl=1;
		// OTOH, structural contacts will have this flag
		else contPhys->isStructural=true;
	}
}




/********************** BrefcomContact ****************************/
CREATE_LOGGER(BrefcomContact);



/********************** BrefcomLaw ****************************/
CREATE_LOGGER(BrefcomLaw);

void BrefcomLaw::calculateNormalForce(){
	Real d=(rbp1->se3.position - rbp2->se3.position).Length();
	//TRVAR2(d,BC->dBreak);
	if(d > BC->dBreak){
		BC->omegaPl=1; /* LOG_DEBUG("Complete damage for #"<<id1<<"+#"<<id2); */
		Fn=Vector3r::ZERO; return; }
	Fn = (d - BC->d0_curr) * BC->Kn * contGeom->normal;
	//TRVAR4(d,BC->d0_curr,Fn.Length(),BC->FnMax_curr);

	BREFREC(d);
	BREFREC(BC->d0_curr);

	#if 0
		if(d>BC->dPeak_curr){ /* on the softening branch of the diagram */
			force+=(BC->dBreak - d)*(BC->Kn / BC->zeta) * contGeom->normal;
			BC->omegaPl=(d - BC->dPeak)/(BC->dBreak - BC->dPeak);
		} else { /* on the elastic branch */
			force+=(d - BC->d0_curr)* BC->Kn * contGeom->normal;
		}
	#endif
}

void BrefcomLaw::calculateShearForce(){
	Fs=Vector3r::ZERO; return;
	
	Fs = BC->Fs;
	Real dt=Omega::instance().getTimeStep();
	// rotation of the contact normal
	Fs -= BC->Fs.Cross( BC->prevNormal.Cross(contGeom->normal) );
	//TRVAR1(BC->Fs.Cross( BC->prevNormal.Cross(contGeom->normal)));
	// mutual rotation of elements
	Real angle=dt*.5*contGeom->normal.Dot(rbp1->angularVelocity+rbp2->angularVelocity);
	Fs -= BC->Fs.Cross(angle*contGeom->normal);
	//TRVAR1(BC->Fs.Cross(angle*contGeom->normal));
	// element slip
	Vector3r relVelocity=
		rbp2->velocity+rbp2->angularVelocity.Cross(contGeom->contactPoint-rbp2->se3.position)-
		rbp1->velocity+rbp1->angularVelocity.Cross(contGeom->contactPoint-rbp1->se3.position);
	Vector3r shearDisplacement=dt*(relVelocity-contGeom->normal.Dot(relVelocity)*contGeom->normal);
	Fs -= BC->Ks*shearDisplacement;
	//TRVAR1(BC->Ks*shearDisplacement);
	if(isnan(Fs[0]) || isnan(Fs[1]) || isnan(Fs[2])){
		TRVAR4(BC->Fs, BC->prevNormal, contGeom->normal, BC->Fs.Cross(BC->prevNormal.Cross(contGeom->normal)));
		TRVAR4(angle, rbp1->angularVelocity, rbp2->angularVelocity,BC->Fs.Cross(angle*contGeom->normal));
		TRVAR4(rbp1->se3.position, rbp2->se3.position,rbp1->velocity,rbp2->velocity);
		TRVAR4(relVelocity,shearDisplacement,BC->Ks,BC->Ks*shearDisplacement);
		exit(0);
	}
}

void BrefcomLaw::rotationEffects(){
	/* TODO */
	/* is in ElasticCohesiveLaw under if(momentRotationLaw) { … }, but that code is messy;
	 * should be rewritten using some article */
}

void BrefcomLaw::envelopeAndDamage(void){

	/* Fn and Fs envelope is:
		Fn < FnMax_curr
		Fs < cohesion_curr - Fn*tan(frictionAngle);

		If any of these conditions is violated, decrement forces keeping their ratio and incement damage.
	*/
	Real fn=Fn.Dot(contGeom->normal) /* sign does matter here */, fs=Fs.Length() /* sign irrelevant */;
	if(fn>Mathr::EPSILON && fn > BC->FnMax_curr){
		/* return force on the softening branch */
		Real dmgFactor=max(1.,(BC->FnMax_curr - (fn - BC->FnMax_curr)/BC->zeta) / fn);
		//if(isnan(dmgFactor)){TRVAR3(fn,Fn,contGeom->normal); exit(0);}
		//TRVAR4(fn,BC->FnMax_curr,dmgFactor,BC->omegaPl);
		Fn*=dmgFactor; fn*=dmgFactor;
		BC->omegaPl=1-fn/BC->FnMax; // FnMax_curr will be updated from omegaPl at the next iteration
		// LOG_DEBUG("Increasing damage to "<<BC->omegaPl);
		if(Omega::instance().getCurrentIteration()%100 && BC->omegaPl>=1) { Real d=(rbp1->se3.position - rbp2->se3.position).Length(); TRVAR4(rbp1->se3.position,rbp2->se3.position,d,fn);  }
		if(isnan(Fn[0]) || isnan(Fn[1]) || isnan(Fn[2])){ TRVAR4(Fn,fn,dmgFactor,BC->omegaPl);	exit(0); }
	}
	//TRVAR2(fs,BC->cohesion_curr - fn*tan(BC->frictionAngle));
	if(fs > BC->cohesion_curr - fn*tan(BC->frictionAngle)){
		Real FsMax_curr=BC->cohesion_curr - fn*tan(BC->frictionAngle);
		Fs.Normalize();
		Fs*=FsMax_curr;
		#if 0
			Real FsMax_undamaged=BC->cohesion - fn*tan(BC->frictionAngle);
			Fs*=(FsMax_curr/fs);
			BC->omegaPl=1-FsMax_curr/FsMax_undamaged;
			TRVAR5(fs,BC->cohesion_curr,FsMax_curr,FsMax_curr/fs,BC->omegaPl);
		#endif
		#if 0 // FIXME: not tested!!!
			/* decrease both forces, keep their proportion; tricky */
			Real fnEnvelope=BC->cohesion_curr/((fn/fs)+tan(BC->frictionAngle)); // normal force on the envelope, with the same Fn/Fs proportion
			dmgFactor=(fnEnvelope-(fn-fnEnvelope)/BC->zeta)/fn; // factor by which decrease Fn
			TRVAR2(fn,fs);
			TRVAR5(BC->cohesion_curr,BC->frictionAngle,dmgFactor,fnEnvelope,BC->omegaPl);
			Fn*=dmgFactor; fn*=dmgFactor;
			Fs*=dmgFactor;
			BC->omegaPl*=(1-fn/fnEnvelope); // increase damage
		#endif
	}
	BREFREC(BC->omegaPl);
	BREFREC(Fn.Length());
	BREFREC(Fs.Length());
}

void BrefcomLaw::applyForces(){
	BC->Fs=Fs; BC->Fn=Fn; // remember forces
	Vector3r force=Fn+Fs;

	static_pointer_cast<Force>(rootBody->physicalActions->find(id1,ForceClassIndex))->force-=force;
	static_pointer_cast<Force>(rootBody->physicalActions->find(id2,ForceClassIndex))->force+=force;
	static_pointer_cast<Momentum>(rootBody->physicalActions->find(id1,MomentumClassIndex))->momentum-=
		(contGeom->contactPoint - rbp1->se3.position).Cross(force);
	static_pointer_cast<Momentum>(rootBody->physicalActions->find(id2,MomentumClassIndex))->momentum+=
		(contGeom->contactPoint - rbp2->se3.position).Cross(force);
}

void BrefcomLaw::action(Body* body){
	rootBody=YADE_CAST<MetaBody*>(body);
	for(InteractionContainer::iterator I=rootBody->transientInteractions->begin(); I!=rootBody->transientInteractions->end(); ++I){
		if(!(*I)->isReal) continue;
		//TRACE;
		// initialize temporaries
		id1=(*I)->getId1(); id2=(*I)->getId2();
		body1=Body::byId(id1); body2=Body::byId(id2);
		assert(body1); assert(body2);
		BC=YADE_PTR_CAST<BrefcomContact>((*I)->interactionPhysics);
		contGeom=YADE_PTR_CAST<SpheresContactGeometry>((*I)->interactionGeometry);
		rbp1=YADE_PTR_CAST<RigidBodyParameters>(body1->physicalParameters);
		rbp2=YADE_PTR_CAST<RigidBodyParameters>(body2->physicalParameters);
		assert(BC); assert(contGeom); assert(rbp1); assert(rbp2);

		// don't disregard links with omegaPl=1, they can still be compressed !!
		// if(BC->omegaPl >= 1) continue; // we want to keep broken links formally alive, for statistics of material damage
		
		recValues.clear(); recLabels.clear();
		BREFREC(id1);
		BREFREC(id2);
		BREFREC2(rbp1->se3.position[0],"x1");
		BREFREC2(rbp1->se3.position[1],"y1");
		BREFREC2(rbp1->se3.position[2],"z1");
		BREFREC2(rbp2->se3.position[0],"x2");
		BREFREC2(rbp2->se3.position[1],"y2");
		BREFREC2(rbp2->se3.position[2],"z2");

		BC->deduceOtherParams(); // initialize non-fundamental params/variables
		//TRVAR5(BC->d0,BC->Kn,BC->Ks,BC->zeta,BC->FnMax);
		//TRVAR5(BC->omegaPl,BC->dBreak,BC->dPeak,BC->d0_curr,BC->dPeak_curr);

		calculateNormalForce();
		calculateShearForce();
		// rotationEffects();

		envelopeAndDamage();
	
		/* see above: we keep even broken links there */
		//if(BC->omegaPl>=1) { (*I)->isReal=false; return; } // if total damage, cancel interaction; collider will delete it properly

		applyForces();

		for(size_t i=0; i<recValues.size(); i++) recStream<<recLabels[i]<<": "<<recValues[i]<<" ";
		recStream<<endl;
	}
}


