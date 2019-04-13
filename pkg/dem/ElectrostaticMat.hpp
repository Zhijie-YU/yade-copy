// 2016 © William Chèvremont <william.chevremont@univ-grenoble-alpes.fr>

#pragma once

#include<pkg/dem/CohesiveFrictionalContactLaw.hpp>
#include<pkg/common/Dispatching.hpp>
#include<pkg/common/Sphere.hpp>
#include<pkg/common/PeriodicEngines.hpp>
#include<pkg/common/NormShearPhys.hpp>
#include<pkg/dem/DemXDofGeom.hpp>
#include<pkg/dem/ScGeom.hpp>
#include<pkg/dem/FrictPhys.hpp>
#include<pkg/dem/ElasticContactLaw.hpp>
#include<pkg/dem/Lubrication.hpp>

namespace py=boost::python;

class ElectrostaticPhys: public LubricationPhys {
        public:
				explicit ElectrostaticPhys(LubricationPhys const&); // Inheritance constructor
                YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(ElectrostaticPhys,LubricationPhys,"IPhys class containing parameters of DLVO interaction Inherits from LubricationPhys. Used by :yref:`Law2_ScGeom_ElectrostaticPhys`.",
                        ((Real,DebyeLength,1e-6,Attr::readonly,"Debye Length $\\kappa^{-1}$[m]"))
                        ((Real,Z,1e-12,Attr::readonly,"Double layer interaction constant $Z$ [N]"))
						((Vector3r,normalDLForce,Vector3r::Zero(),Attr::readonly,"Normal force due to Double Layer electrostatic repulsion"))
			, // ctors
                        createIndex();,
		);
		DECLARE_LOGGER;
                REGISTER_CLASS_INDEX(ElectrostaticPhys,LubricationPhys);
};
REGISTER_SERIALIZABLE(ElectrostaticPhys);

class Ip2_FrictMat_FrictMat_ElectrostaticPhys: public Ip2_FrictMat_FrictMat_LubricationPhys{
	public:
		virtual void go(const shared_ptr<Material>& material1, const shared_ptr<Material>& material2, const shared_ptr<Interaction>& interaction);
		static Real getInteractionConstant(Real const& epsr=78, Real const& T=293, Real const& z=1, Real const& phi0=0.050);
		//BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getInteractionConstant_overloads, getInteractionConstant, 0, 4)
                FUNCTOR2D(FrictMat,FrictMat);
		DECLARE_LOGGER;
                YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(Ip2_FrictMat_FrictMat_ElectrostaticPhys, Ip2_FrictMat_FrictMat_LubricationPhys,"Ip2 creating Electrostatic_Phys from two ElectrostaticMat instances.",
                        ((Real,DebyeLength,1.e-6,,"Debye length [m]."))
                        ((Real,Z,1.e-12,,"Interaction constant [N]."))
                    ,,
					.def("getInteractionConstant", &getInteractionConstant, (py::args("epsr")=78,py::args("T")=293,py::args("z")=1,py::args("phi0")=0.050),"Get the interaction constant from thermal properties").staticmethod("getInteractionConstant")
		);
};
REGISTER_SERIALIZABLE(Ip2_FrictMat_FrictMat_ElectrostaticPhys);



class Law2_ScGeom_ElectrostaticPhys: public Law2_ScGeom_ImplicitLubricationPhys{
	public:
		Real normalForce_DL_Adim(ElectrostaticPhys *phys, ScGeom* geom, Real const& undot, bool isNew);

		Real DL_DichoAdimExp_integrate_u(Real const& un, Real const& eps, Real const& alpha, Real const& Z, Real const& K, Real & prevDotU, Real const& dt, Real const& prev_d, Real const& undot);
		Real ObjF(Real const& un, Real const& eps, Real const& alpha, Real const& prevDotU, Real const& dt, Real const& prev_d, Real const& undot, Real const& Z, Real const& K, Real const& d);
		
		static void getStressForEachBody(vector<Matrix3r>& DLStresses);
		static py::tuple PyGetStressForEachBody();
		static void getTotalStresses(Matrix3r& DLStresses);
		static py::tuple PyGetTotalStresses();
		
		bool go(shared_ptr<IGeom>& iGeom, shared_ptr<IPhys>& iPhys, Interaction* interaction);
		FUNCTOR2D(GenericSpheresContact,ElectrostaticPhys);
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(Law2_ScGeom_ElectrostaticPhys,Law2_ScGeom_ImplicitLubricationPhys,"Material law for lubricated spheres with Double Layer interaction between 2 spheres. Solved by dichotomy method. No exact resolution method available",,,
			.def("getStressForEachBody",&Law2_ScGeom_ElectrostaticPhys::PyGetStressForEachBody,"Get stresses tensors for each bodies: normal contact stress, shear contact stress, normal lubrication stress, shear lubrication stress, DL stress.")
			.staticmethod("getStressForEachBody")
			.def("getTotalStresses",&Law2_ScGeom_ElectrostaticPhys::PyGetTotalStresses,"Get total stresses tensors: normal contact stress, shear contact stress, normal lubrication stress, shear lubrication stress, DL stress")
			.staticmethod("getTotalStresses")
		);
		DECLARE_LOGGER;

};
REGISTER_SERIALIZABLE(Law2_ScGeom_ElectrostaticPhys);
