/* CWBoon 2016 */
#ifdef YADE_POTENTIAL_BLOCKS

#pragma once
#ifdef YADE_OPENGL
	#include <pkg/common/GLDrawFunctors.hpp>
#endif

#include <vector>
#include <lib/computational-geometry/MarchingCube.hpp>
#include <pkg/dem/PotentialBlock.hpp>
#include <pkg/dem/PotentialBlock2AABB.hpp>
#include <pkg/common/PeriodicEngines.hpp>

#include <vtkImplicitFunction.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkPolyData.h>

// https://codeyarns.com/2014/03/11/how-to-selectively-ignore-a-gcc-warning/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcomment"
// Code that generates this warning, Note: we cannot do this trick in yade. If we have a warning in yade, we have to fix it! See also https://gitlab.com/yade-dev/trunk/merge_requests/73
// This method will work once g++ bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53431#c34 is fixed.
#include <vtkTriangle.h>
#pragma GCC diagnostic pop

#include <vtkSmartPointer.h>
#include <vtkFloatArray.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkSampleFunction.h>
#include <vtkStructuredPoints.h>
#include <vtkStructuredPointsWriter.h>
#include <vtkWriter.h>
#include <vtkExtractVOI.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkXMLStructuredGridWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkRenderWindowInteractor.h>

#include <vtkActor.h>
#include <vtkAppendPolyData.h>

#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkVectorText.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkTextActor3D.h>
#include <vtkCylinderSource.h>



#ifdef YADE_CGAL
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* INCLUDE STATEMENTS FROM: Polyhedra.hpp file */
// TODO Remember to remove redundant scripts. I don't need all the below #include invocations
#include <core/Omega.hpp>
#include <core/Shape.hpp>
#include <core/Interaction.hpp>
#include <core/Material.hpp>
#include <pkg/dem/ScGeom.hpp>
#include <pkg/dem/FrictPhys.hpp>
#include <pkg/common/Wall.hpp>
#include <pkg/common/Facet.hpp>
#include <pkg/common/Sphere.hpp>
#include <pkg/common/Dispatching.hpp>
#include <pkg/common/ElastMat.hpp>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_data_structure_3.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_items_with_id_3.h> 

#include <CGAL/Surface_mesh.h>

#include <CGAL/convex_hull_3.h>
#include <CGAL/Tetrahedron_3.h>
#include <CGAL/linear_least_squares_fitting_3.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>
#include <CGAL/squared_distance_3.h>


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* NEW SCRIPT USING FACETS*/

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

//CGAL definitions - does not work with another kernel!! Why???
using K = CGAL::Exact_predicates_inexact_constructions_kernel;
using Polyhedron = CGAL::Polyhedron_3<K>;

using Mesh = CGAL::Surface_mesh<K::Point_3>;

using Triangulation = CGAL::Delaunay_triangulation_3<K>;
using CGALpoint = K::Point_3;
using CGALtriangle = K::Triangle_3;
using CGALvector = K::Vector_3;
using Transformation = CGAL::Aff_transformation_3<K>;
using Segment = K::Segment_3;
using Triangle = CGAL::Triangle_3<K>;
using Plane = CGAL::Plane_3<K>;
using Line = CGAL::Line_3<K>;
using CGAL_ORIGIN = CGAL::Origin;
using CGAL_AABB_tree = CGAL::AABB_tree<CGAL::AABB_traits<K,CGAL::AABB_triangle_primitive<K,std::vector<Triangle>::iterator>>>;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef YADE_OPENGL
	#include<pkg/common/GLDrawFunctors.hpp>
	#include<lib/opengl/OpenGLWrapper.hpp>
	#include<lib/opengl/GLUtils.hpp>
	#include<GL/glu.h>
	#include<pkg/dem/Shop.hpp>
	
	/* Draw PotentialBlocks using OpenGL */
	class Gl1_PotentialBlock: public GlShapeFunctor{	
		public:
			struct TriangulationMatrix{
				vector<Vector3i> triangles;
			};
			static vector<TriangulationMatrix> TM ;


			struct VerticesMatrix{
				vector<Vector3r> v; 	
			};
			static vector<VerticesMatrix> VM ;


			struct CentroidMatrix{      
				Vector3r c;  
			};	
			static vector<CentroidMatrix> CM ;


			virtual void go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&);

			YADE_CLASS_BASE_DOC_STATICATTRS(Gl1_PotentialBlock,GlShapeFunctor,"Renders :yref:`PotentialBlock` object",
				((bool,wire,false,,"Only show wireframe"))
			);
			RENDERS(PotentialBlock);

		protected:
			Polyhedron P;
			Vector3r centroid;
			Real volume; // used to be: vo
			bool init;
	};
	REGISTER_SERIALIZABLE(Gl1_PotentialBlock);

bool P_volume_centroid(Polyhedron P, Real * volume, Vector3r * centroid);
//Polyhedron Simplify(Polyhedron P, Real lim);

#endif // YADE_OPENGL
#endif // YADE_CGAL









/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* PREVIOUSLY EXISTING CODE DEVELOPED BY CW BOON USING THE MARCHING CUBES */
/* TODO  CREATE A BOOLEAN TO CHOOSE BETWEEN display="ACTUAL_PARTICLE" OR display="INNER_PP" */

//#ifdef YADE_OPENGL

//class Gl1_PotentialBlock : public GlShapeFunctor
//{	
//	private :
//		MarchingCube mc;
//		Vector3r Min,Max;
//		vector<vector<vector<Real> > > scalarField,weights;
//		void generateScalarField(const PotentialBlock& pp);
//		void calcMinMax(const PotentialBlock& cm);
//		Vector3r isoStep;
//		Eigen::Matrix3d rotationMatrix;

//		int display;

//	public :
//		struct scalarF{
//			 vector<vector<vector<float> > > scalarField2;
//			 vector<Vector3r> triangles;
//			 vector<Vector3r> normals;
//			 int nbTriangles;
//		};

//		virtual void go(const shared_ptr<Shape>&, const shared_ptr<State>&,bool,const GLViewInfo&);
//		double evaluateF(const PotentialBlock& pp, double x, double y, double z);
//		static vector<scalarF> SF ;
//		//void clearMemory();
//		
//	YADE_CLASS_BASE_DOC_STATICATTRS(Gl1_PotentialBlock,GlShapeFunctor,"Renders :yref:`PotentialBlock` object",
//		((int,sizeX,30,,"Number of divisions in the X direction for triangulation"))
//		((int,sizeY,30,,"Number of divisions in the Y direction for triangulation"))
//		((int,sizeZ,30,,"Number of divisions in the Z direction for triangulation"))
//		((bool,store,true,Attr::hidden,"Whether to store initial triangulation")) //USERS DON'T NEED TO HAVE ACCESS TO THIS
//		((bool,initialized,false,Attr::hidden,"Whether the triangulation has been initialized")) //USERS DON'T NEED TO HAVE ACCESS TO THIS
//		((Real,aabbEnlargeFactor,1.3,,"some factor for displaying algorithm, try different value if you have problems with displaying"))
//		((bool,wire,false,,"Only show wireframe"))
////		((vector<scalarF>,SF,"Scalar field used by the Marching cubes algorithm"))
//	);
//	RENDERS(PotentialBlock);


//};
//REGISTER_SERIALIZABLE(Gl1_PotentialBlock);

//#endif // YADE_OPENGL




	class ImpFuncPB : public vtkImplicitFunction {
		public:
		    vtkTypeMacro(ImpFuncPB,vtkImplicitFunction);
		    //void PrintSelf(ostream& os, vtkIndent indent);

		    // Create a new function
		    static ImpFuncPB * New(void);
		    vector<double>a; vector<double>b; vector<double>c; vector<double>d;
		    double k; double r; double R; Eigen::Matrix3d rotationMatrix;
		    bool clump;
		    double clumpMemberCentreX;
		    double clumpMemberCentreY;
		    double clumpMemberCentreZ;

		    // Evaluate function
		    double FunctionValue(double x[3]);
		    double EvaluateFunction(double x[3]){ 
			//return this->vtkImplicitFunction::EvaluateFunction(x);
			return FunctionValue(x);
		    };
		    
		    double EvaluateFunction(double x, double y, double z) {
			return this->vtkImplicitFunction::EvaluateFunction(x, y, z);
		    };

		    // Evaluate gradient for function
		   void EvaluateGradient(double x[3], double n[3]){ };

		    // If you need to set parameters, add methods here

		protected:
		   ImpFuncPB();
		   ~ImpFuncPB();
		   ImpFuncPB(const ImpFuncPB&) {}
		   void operator=(const ImpFuncPB&) {}

		    // Add parameters/members here if you need
	};

	/* PotentialBlockVTKRecorder */
	class PotentialBlockVTKRecorder: public PeriodicEngine{	
	  public:
		vtkSmartPointer<ImpFuncPB> function;
		
	  virtual void action(void);
	  YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(PotentialBlockVTKRecorder,PeriodicEngine,"Engine recording potential blocks as surfaces into files with given periodicity.",
		((string,fileName,,,"File prefix to save to"))
		((int,sampleX,30,,"size of contact point"))
		((int,sampleY,30,,"size of contact point"))
		((int,sampleZ,30,,"size of contact point"))
		((double,maxDimension,30,,"size of contact point"))
		((bool,twoDimension,false,,"size of contact point"))
		((bool,REC_INTERACTION,false,,"contact point and forces"))
		((bool,REC_COLORS,false,,"colors"))
		((bool,REC_VELOCITY,false,,"velocity"))
		((bool,REC_ID,true,,"id"))
		,
		function = ImpFuncPB::New();
		,
		
	  );
	};
	REGISTER_SERIALIZABLE(PotentialBlockVTKRecorder);


	/* PotentialBlockVTKRecorderTunnel */
	class PotentialBlockVTKRecorderTunnel: public PeriodicEngine{	
	  public:
		vtkSmartPointer<ImpFuncPB> function;
		
	  virtual void action(void);
	  YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(PotentialBlockVTKRecorderTunnel,PeriodicEngine,"Engine recording potential blocks as surfaces into files with given periodicity.",
		((string,fileName,,,"File prefix to save to"))
		((int,sampleX,30,,"size of contact point"))
		((int,sampleY,30,,"size of contact point"))
		((int,sampleZ,30,,"size of contact point"))
		((double,maxDimension,30,,"size of contact point"))
		((bool,twoDimension,false,,"size of contact point"))
		((bool,REC_INTERACTION,false,,"contact point and forces"))
		((bool,REC_COLORS,false,,"colors"))
		((bool,REC_VELOCITY,false,,"velocity"))
		((bool,REC_ID,true,,"id"))
		,
		function = ImpFuncPB::New();
		,
		
	  );
	};
	REGISTER_SERIALIZABLE(PotentialBlockVTKRecorderTunnel);


#endif // YADE_POTENTIAL_BLOCKS




