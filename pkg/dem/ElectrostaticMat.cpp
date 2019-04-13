// 2016 © William Chèvremont <william.chevremont@univ-grenoble-alpes.fr>

#include"ElectrostaticMat.hpp"


YADE_PLUGIN((Ip2_FrictMat_FrictMat_ElectrostaticPhys)(ElectrostaticPhys)(Law2_ScGeom_ElectrostaticPhys))

// Inheritance constructor
ElectrostaticPhys::ElectrostaticPhys(LubricationPhys const& obj) :
	LubricationPhys(obj),
	DebyeLength(1.e-6),
	Z(1.e-12)
{
}

CREATE_LOGGER(ElectrostaticPhys);

// Calculation on interaction constant based on physical properties
Real Ip2_FrictMat_FrictMat_ElectrostaticPhys::getInteractionConstant(Real const& epsr, Real const& T, Real const& z, Real const& phi0)
{
	// Physical constants
	const Real kB(1.38064852e-23); /* J/K Boltzmann*/
	//const Real nA(6.02214086e26); /* 1/kg Avogadro*/
	const Real e(1.60217662e-19); /* A.s Electron charge*/
	const Real VacPerm(8.854187817e-12); /* F/m Permittivity of vacuum*/
	
	return 64.*M_PI*epsr*VacPerm*std::pow(kB*T/e*std::tanh(z*e*phi0/(4.*kB*T)),2);
}

void Ip2_FrictMat_FrictMat_ElectrostaticPhys::go(const shared_ptr<Material>& material1, const shared_ptr<Material>& material2, const shared_ptr<Interaction>& interaction)
{
	if (interaction->phys) return;

	// Inheritance & cast
	Ip2_FrictMat_FrictMat_LubricationPhys::go(material1,material2,interaction);
	LubricationPhys* ph = YADE_CAST<LubricationPhys*>(interaction->phys.get());
	shared_ptr<ElectrostaticPhys> phys(new ElectrostaticPhys(*ph));
	interaction->phys = phys;
	
	phys->DebyeLength = DebyeLength;
	phys->Z = Z;
}
CREATE_LOGGER(Ip2_ElectrostaticMat_ElectrostaticMat_ElectrostaticPhys);


/********************** Law2_ScGeom_ElectrostaticPhys ****************************/

Real Law2_ScGeom_ElectrostaticPhys::normalForce_DL_Adim(ElectrostaticPhys* phys, ScGeom* geom, Real const& undot, bool isNew)
{
	// Dry contact
	if(phys->nun <= 0.) {
		if(!warnedOnce) LOG_WARN("Can't solve with dimentionless-exponential method without fluid!");
		warnedOnce = true;
		phys->u = -1;
		return -1; }
	
	Real a((geom->radius1+geom->radius2)/2.);
	if(isNew) { phys->u = -geom->penetrationDepth-undot*scene->dt; phys->delta = std::log(phys->u/a); }
	
	Real d = DL_DichoAdimExp_integrate_u(-geom->penetrationDepth/a, 2.*phys->eps, 1.0, phys->Z/(phys->kn*a), a/phys->DebyeLength, phys->prevDotU, scene->dt*a*phys->kn/phys->nun, phys->delta, phys->nun/phys->kn/std::pow(a,2)*undot); // Solution by dichotomy
	
	phys->normalForce = phys->kn*(-geom->penetrationDepth-a*std::exp(d))*geom->normal;
	phys->normalContactForce = (phys->nun > 0.) ? Vector3r(-phys->kn*(std::max(2.*a*phys->eps-a*std::exp(d),0.))*geom->normal) : phys->normalForce;
	
	phys->normalDLForce = (- phys->Z*a/phys->DebyeLength*std::exp(-a*std::exp(d)/phys->DebyeLength))*geom->normal;
	phys->normalLubricationForce = phys->kn*a*phys->prevDotU*geom->normal;
	
	phys->delta = d;
	phys->u = a*std::exp(d);
	
	phys->contact = phys->normalContactForce.norm() != 0;
	phys->ue = -geom->penetrationDepth - phys->u;
	
	return phys->u;
}

// Dimentionless Dichotomy solver
Real Law2_ScGeom_ElectrostaticPhys::DL_DichoAdimExp_integrate_u(Real const& un, Real const& eps, Real const& alpha, Real const& Z, Real const& K, Real & prevDotU, Real const& dt, Real const& prev_d, Real const& undot)
{
	Real F = 0.;
	Real d_left(prev_d-1.), d_right(prev_d+1.);
	Real F_left(ObjF(un, eps, alpha, prevDotU, dt, prev_d, undot, Z, K, d_left));
	Real F_right(ObjF(un, eps, alpha, prevDotU, dt, prev_d, undot, Z, K, d_right));
	Real d;
	
	// Init: search for interval that contain sign change
	Real inc = (F_left < 0.) ? 1. : -1;
	inc = (F_left < F_right) ? inc : -inc;
	while(F_left*F_right >= 0. && std::isfinite(F_left) && std::isfinite(F_right)) {
		d_left += inc;
		d_right += inc;
		F_left = ObjF(un, eps, alpha, prevDotU, dt, prev_d, undot, Z, K, d_left);
		F_right = ObjF(un, eps, alpha, prevDotU, dt, prev_d, undot, Z, K, d_right);
	}
	
	if((!std::isfinite(F_left) || !std::isfinite(F_right))) {
		if(debug) LOG_WARN("Wrong direction");
		inc = -inc; // RE-INIT
		d_left = prev_d-1.;
		d_right = prev_d+1.;
		
		while(F_left*F_right >= 0. && std::isfinite(F_left) && std::isfinite(F_right)) {
			d_left += inc;
			d_right += inc;
			F_left = ObjF(un, eps, alpha, prevDotU, dt, prev_d, undot, Z, K, d_left);
			F_right = ObjF(un, eps, alpha, prevDotU, dt, prev_d, undot, Z, K, d_right);
		}
	}
	
	if((!std::isfinite(F_left) || !std::isfinite(F_right)))
		LOG_ERROR("Initial point problem!! d_left=" << d_left << " F_left=" << F_left << " d_right=" << d_right << " F_right=" << F_right);
	
	// Iterate to find the zero.
	int i;
	for(i=0;i<MaxIter;i++) {
		if(F_left*F_right > 0.)
			LOG_ERROR("Both function have same sign!! d_left=" << d_left << " F_left=" << F_left << " d_right=" << d_right << " F_right=" << F_right);
		
		//d = (d_left + d_right)/2.; // Dichotomy
		d = d_left - F_left*(d_right - d_left)/(F_right - F_left); // Regula Falsi
		F = ObjF(un, eps, alpha, prevDotU, dt, prev_d, undot, Z, K, d);
		
		if(std::abs(F) < SolutionTol)
			break;
		
		if(F*F_left < 0.) {
			F_right = F;
			d_right = d;
		} else {
			F_left = F;
			d_left = d;
		}
	}
	
	if(i == MaxIter)
		LOG_WARN("Max iteration reach: d_left=" << d_left << " F_left=" << F_left << " d_right=" << d_right << " F_right=" << F_right);
	
	Real a = (std::exp(d) < eps) ? alpha : 0.;
	prevDotU = -(1.+a)*std::exp(d) + a*eps + un + Z*K*std::exp(-K*std::exp(d));
	
	return d;
}

Real Law2_ScGeom_ElectrostaticPhys::ObjF(Real const& un, Real const& eps, Real const& alpha, Real const& prevDotU, Real const& dt, Real const& prev_d, Real const& undot, Real const& Z, Real const& K, Real const& d)
{
	Real a = (std::exp(d) < eps) ? alpha : 0.;
	
	return theta*(un - std::exp(d)*(1.+a) + a*eps + K*Z*std::exp(-K*std::exp(d))) + (1.-theta)*prevDotU*std::exp(prev_d-d) - 1./dt*(1.-std::exp(prev_d-d));
	
	//return theta*(-(1.+a)*std::exp(d) + a*eps+un) + (1.-theta)*prevDotU*std::exp(prev_d-d) - 1./dt*(1. - std::exp(prev_d-d));
}

bool Law2_ScGeom_ElectrostaticPhys::go(shared_ptr<IGeom>& iGeom, shared_ptr<IPhys>& iPhys, Interaction* interaction)
{
	// Physic
    ElectrostaticPhys* phys=static_cast<ElectrostaticPhys*>(iPhys.get());

    // Geometry
    ScGeom* geom=static_cast<ScGeom*>(iGeom.get());

    // Get bodies properties
    Body::id_t id1 = interaction->getId1();
    Body::id_t id2 = interaction->getId2();
    const shared_ptr<Body> b1 = Body::byId(id1,scene);
    const shared_ptr<Body> b2 = Body::byId(id2,scene);
    State* s1 = b1->state.get();
    State* s2 = b2->state.get();

    // geometric parameters
    Real a((geom->radius1+geom->radius2)/2.);
    bool isNew=false;
	
	// Speeds
    Vector3r shiftVel=scene->isPeriodic ? Vector3r(scene->cell->velGrad*scene->cell->hSize*interaction->cellDist.cast<Real>()) : Vector3r::Zero();
    Vector3r shift2 = scene->isPeriodic ? Vector3r(scene->cell->hSize*interaction->cellDist.cast<Real>()): Vector3r::Zero();

    Vector3r relV = geom->getIncidentVel(s1, s2, scene->dt, shift2, shiftVel, false );
	Real undot = relV.dot(geom->normal); // Normal velocity norm
	
    if(-geom->penetrationDepth > a && -geom->penetrationDepth > 10.*phys->DebyeLength) { return false; }
    
    // inititalization
	if(phys->u == -1. ) {phys->u = -geom->penetrationDepth; isNew=true;}
	
	// Solve normal
	normalForce_DL_Adim(phys,geom, undot,isNew);
	
	// Solve shear and torques
	Vector3r C1 = Vector3r::Zero();
	Vector3r C2 = Vector3r::Zero();
	
	computeShearForceAndTorques_log(phys, geom, s1, s2, C1, C2);
	
    // Apply!
    scene->forces.addForce(id1,phys->normalForce+phys->shearForce);
    scene->forces.addTorque(id1,C1);

    scene->forces.addForce(id2,-(phys->normalForce+phys->shearForce));
    scene->forces.addTorque(id2,C2);

    return true;
}

void Law2_ScGeom_ElectrostaticPhys::getStressForEachBody(vector<Matrix3r>& DLStresses)
{
	const shared_ptr<Scene>& scene=Omega::instance().getScene();
	DLStresses.resize(scene->bodies->size());
	for (size_t k=0;k<scene->bodies->size();k++) {
		DLStresses[k]=Matrix3r::Zero();
	}
	
	FOREACH(const shared_ptr<Interaction>& I, *scene->interactions) {
		if(!I->isReal()) continue;
		GenericSpheresContact* geom=YADE_CAST<GenericSpheresContact*>(I->geom.get());
		ElectrostaticPhys* phys=YADE_CAST<ElectrostaticPhys*>(I->phys.get());
		
		if(phys) {
			Vector3r lV1 = (3.0/(4.0*Mathr::PI*pow(geom->refR1,3)))*((geom->contactPoint-Body::byId(I->getId1(),scene)->state->pos));
			Vector3r lV2 = Vector3r::Zero();
			if (!scene->isPeriodic)
				lV2 = (3.0/(4.0*Mathr::PI*pow(geom->refR2,3)))*((geom->contactPoint- (Body::byId(I->getId2(),scene)->state->pos)));
			else
				lV2 = (3.0/(4.0*Mathr::PI*pow(geom->refR2,3)))*((geom->contactPoint- (Body::byId(I->getId2(),scene)->state->pos + (scene->cell->hSize*I->cellDist.cast<Real>()))));

			DLStresses[I->getId1()] += phys->normalDLForce*lV1.transpose();
			DLStresses[I->getId2()] -= phys->normalDLForce*lV2.transpose();
		}
	}
}

py::tuple Law2_ScGeom_ElectrostaticPhys::PyGetStressForEachBody()
{
	py::list nc, sc, nl, sl, nd;
	vector<Matrix3r> NCs, SCs, NLs, SLs, NDs;
	Law2_ScGeom_ImplicitLubricationPhys::getStressForEachBody(NCs, SCs, NLs, SLs);
	getStressForEachBody(NDs);
	FOREACH(const Matrix3r& m, NCs) nc.append(m);
	FOREACH(const Matrix3r& m, SCs) sc.append(m);
	FOREACH(const Matrix3r& m, NLs) nl.append(m);
	FOREACH(const Matrix3r& m, SLs) sl.append(m);
	FOREACH(const Matrix3r& m, NDs) nd.append(m);
	return py::make_tuple(nc, sc, nl, sl, nd);
}

void Law2_ScGeom_ElectrostaticPhys::getTotalStresses(Matrix3r& DLStresses)
{
	vector<Matrix3r> NDs;
	getStressForEachBody(NDs);
	
	DLStresses = Matrix3r::Zero();
    
  	const shared_ptr<Scene>& scene=Omega::instance().getScene();
    
    if(!scene->isPeriodic) {
        LOG_ERROR("This method can only be used in periodic simulations");
        return;
    }
    
    for(unsigned int i(0);i<NDs.size();i++) {
        Sphere * s = YADE_CAST<Sphere*>(Body::byId(i,scene)->shape.get());
        
        if(s) {
            Real vol = 4./3.*M_PI*pow(s->radius,3);
            
            DLStresses += NDs[i]*vol;
        }
    }
    
    DLStresses /= scene->cell->getVolume();
}

py::tuple Law2_ScGeom_ElectrostaticPhys::PyGetTotalStresses()
{
	Matrix3r nc(Matrix3r::Zero()), sc(Matrix3r::Zero()), nl(Matrix3r::Zero()), sl(Matrix3r::Zero()), nd(Matrix3r::Zero());

    Law2_ScGeom_ImplicitLubricationPhys::getTotalStresses(nc, sc, nl, sl);
	getTotalStresses(nd);
	return py::make_tuple(nc, sc, nl, sl, nd);
}

CREATE_LOGGER(Law2_ScGeom_ElectrostaticPhys);
