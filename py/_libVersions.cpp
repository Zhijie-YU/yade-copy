#include <core/Omega.hpp>
#include <lib/pyutil/doc_opts.hpp>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/python/tuple.hpp>

namespace py = boost::python;

// I used the list in https://yade-dem.org/doc/installation.html#prerequisites
// If we need a version of some library not listed in doc/sphinx/installation.rst, then it must also be added to that list!

// 1. compiler
// https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
#ifdef __GNUC__
	py::list gccVer() {
		py::list ret;
		ret.append( py::make_tuple( __GNUC__   , __GNUC_MINOR__ , __GNUC_PATCHLEVEL__ ));
		std::string prefix{};
#ifndef BOOST_GCC
// https://www.boost.org/doc/libs/1_44_0/libs/config/doc/html/boost_config/boost_macro_reference.html#boost_config.boost_macro_reference.boost_informational_macros
		// use boost macro to detect if it's really a gcc compiler.
		prefix += "not_gcc_";
#endif
#ifdef BOOST_INTEL
		prefix += "INTEL_";
#endif
#ifdef BOOST_CLANG
		prefix += "CLANG_";
#endif
		ret.append( prefix+boost::lexical_cast<std::string>(__GNUC__)+"."+boost::lexical_cast<std::string>(__GNUC_MINOR__)+"."+boost::lexical_cast<std::string>(__GNUC_PATCHLEVEL__ ));
		return ret;
	}
#else
	py::list gccVer() { return {}; }
#endif

// http://clang.llvm.org/docs/LanguageExtensions.html#builtin-macros
#ifdef __clang__
	py::list clangVer() {
		py::list ret;
		ret.append( py::make_tuple( __clang_major__ , __clang_minor__ , __clang_patchlevel__ ));
		ret.append( boost::lexical_cast<std::string>( __clang_version__ ));
		return ret;
	}
#else
	py::list clangVer() { return {}; }
#endif

// 2. Boost
// https://www.boost.org/doc/libs/1_44_0/libs/config/doc/html/boost_config/boost_macro_reference.html#boost_config.boost_macro_reference.boost_informational_macros
	py::list boostVer() {
		py::list ret;
		ret.append( py::make_tuple( BOOST_VERSION / 100000 , BOOST_VERSION / 100 % 1000 , BOOST_VERSION % 100 ));
// that's the default BOOST_VERSION
//		ret.append( boost::lexical_cast<std::string>( BOOST_VERSION ));
// But the string with dots is a bit more readable
		ret.append( boost::lexical_cast<std::string>(BOOST_VERSION / 100000)+"."+boost::lexical_cast<std::string>(BOOST_VERSION / 100 % 1000)+"."+boost::lexical_cast<std::string>(BOOST_VERSION % 100) );
		return ret;
	}

// 3. qt
// https://doc.qt.io/qt-5/qtglobal.html#QT_VERSION_STR
// https://doc.qt.io/archives/qt-4.8/qtglobal.html#QT_VERSION_STR
#if defined(YADE_QT4) || defined(YADE_QT5)
	#include <QtGlobal>
	py::list qtVer() {
		py::list ret;
		// these were introduced in qt5
		//ret.append( py::make_tuple( QT_VERSION_MAJOR , QT_VERSION_MINOR , QT_VERSION_PATCH ));
		// so I have to use the old version macro.
		ret.append( py::make_tuple( QT_VERSION / 65536 , QT_VERSION / 256 % 256 , QT_VERSION % 256 ));
		ret.append( QT_VERSION_STR );
		return ret;
	}
#else
	py::list qtVer() { return {}; }
#endif

// 4. freeglut
#ifdef YADE_OPENGL
	// NOTE: maybe we should someday switch from freeglut3-dev to libglfw3-dev, https://www.glfw.org/
	#include <GL/freeglut.h> // debian package 2.8.1-3
	py::list freeglutVer() {
		py::list ret;
		// I couldn't find anything that would return the actual debian package version. So this returns 13.4.1, which is weird.
		ret.append( py::make_tuple(                  GLUT_XLIB_IMPLEMENTATION   ,                                   GLUT_API_VERSION   ,                                   FREEGLUT));
		ret.append( boost::lexical_cast<std::string>(GLUT_XLIB_IMPLEMENTATION)+"."+boost::lexical_cast<std::string>(GLUT_API_VERSION)+"."+boost::lexical_cast<std::string>(FREEGLUT));
		return ret;
	}
	#include <GL/glext.h>    // debian package 13.0.6
	py::list glVer() {
		py::list ret;
		// I couldn't find anything that would return the actual debian package version. So this returns 2016.07.14, which is weird.
		ret.append( py::make_tuple( GL_GLEXT_VERSION / 10000 , GL_GLEXT_VERSION / 100 % 100 , GL_GLEXT_VERSION % 100 ));
		ret.append( boost::lexical_cast<std::string>(GL_GLEXT_VERSION));
		return ret;
	}
// 5. qglviewer
	#include <QGLViewer/config.h>
	py::list qglviewerVer() {
		py::list ret;
		ret.append( py::make_tuple(                  QGLVIEWER_VERSION / 65536   ,                                   QGLVIEWER_VERSION / 256 % 256   ,                                   QGLVIEWER_VERSION % 256 ));
		ret.append( boost::lexical_cast<std::string>(QGLVIEWER_VERSION / 65536)+"."+boost::lexical_cast<std::string>(QGLVIEWER_VERSION / 256 % 256)+"."+boost::lexical_cast<std::string>(QGLVIEWER_VERSION % 256 ));
		return ret;
	}
#else
	py::list freeglutVer() { return {}; }
	py::list glVer() { return {}; }
	py::list qglviewerVer() { return {}; }
#endif

// 6. python
	py::list pythonVer() {
		py::list ret;
		ret.append( py::make_tuple(                  PY_MAJOR_VERSION   ,                                   PY_MINOR_VERSION   ,                                   PY_MICRO_VERSION ));
		ret.append( boost::lexical_cast<std::string>(PY_MAJOR_VERSION)+"."+boost::lexical_cast<std::string>(PY_MINOR_VERSION)+"."+boost::lexical_cast<std::string>(PY_MICRO_VERSION ));
		return ret;
	}

// 7. matplotlib (I don't think this could be detected from inside C++)

// 8. eigen

	#include <Eigen/Core>
	py::list eigenVer() {
		py::list ret;
		ret.append( py::make_tuple(                  EIGEN_WORLD_VERSION ,                                     EIGEN_MAJOR_VERSION ,                                     EIGEN_MINOR_VERSION));
		ret.append( boost::lexical_cast<std::string>(EIGEN_WORLD_VERSION)+"."+boost::lexical_cast<std::string>(EIGEN_MAJOR_VERSION)+"."+boost::lexical_cast<std::string>(EIGEN_MINOR_VERSION));
		return ret;
	}

// 9. gdb (I don't think this could be detected from inside C++)

// 10. sqlite3
	#include <sqlite3.h>
	py::list sqliteVer() {
		py::list ret;
		ret.append( py::make_tuple( SQLITE_VERSION_NUMBER / 1000000 , SQLITE_VERSION_NUMBER / 1000 % 1000 , SQLITE_VERSION_NUMBER % 1000 ));
		ret.append( boost::lexical_cast<std::string>(SQLITE_VERSION));
		return ret;
	}

// 11. Loki - very strange, but this library has no version inside header files. This doesn't mattery anyway - latest version is from year 2011.


#ifdef YADE_OPENMP
// https://www.openmp.org/specifications/
// lib/base/openmp-accu.hpp
// it is not in doc/sphinx/installation.rst
// Hmm, it seems that OpenMP version is connected with g++ version. So knowing compiler version is probably enough.
#endif

// 12. VTK
#ifdef YADE_VTK
	#include <vtkVersion.h>
	py::list vtkVer() {
		py::list ret;
		ret.append( py::make_tuple (VTK_MAJOR_VERSION , VTK_MINOR_VERSION , VTK_BUILD_VERSION ));
		ret.append( boost::lexical_cast<std::string>(VTK_VERSION) );
		return ret;
	}
#else
        py::list vtkVer() { return {}; }
#endif

// 13. CGAL
#ifdef YADE_CGAL
	#include <CGAL/version_macros.h>
	py::list cgalVer() {
		py::list ret;
		ret.append( py::make_tuple( CGAL_VERSION_MAJOR , CGAL_VERSION_MINOR , CGAL_VERSION_PATCH ) );
		ret.append( boost::lexical_cast<std::string>(CGAL_VERSION_STR) );
		return ret;
	}
#else
	py::list cgalVer() { return {}; }
#endif

// 14. SuiteSparse
#ifdef LINSOLV
	#include <SuiteSparse_config.h>
	py::list suitesparseVer() {
		py::list ret;
		ret.append( py::make_tuple(                  SUITESPARSE_MAIN_VERSION   ,                                   SUITESPARSE_SUB_VERSION   ,                                   SUITESPARSE_SUBSUB_VERSION));
		ret.append( boost::lexical_cast<std::string>(SUITESPARSE_MAIN_VERSION)+"."+boost::lexical_cast<std::string>(SUITESPARSE_SUB_VERSION)+"."+boost::lexical_cast<std::string>(SUITESPARSE_SUBSUB_VERSION));
		return ret;
	}
#else
	py::list suitesparseVer() { return {}; }
#endif

#if defined(LINSOLV) || defined(YADE_POTENTIAL_PARTICLES) || defined(YADE_POTENTIAL_BLOCKS) || defined(FLOW_ENGINE)
// 15. openblas
	// if this include does not work then add relevant path specific to your system into file cMake/FindOpenBlas.cmake
	#include <openblas_config.h>
	py::list openblasVer() {
		py::list ret;
		ret.append( py::make_tuple( 0 , 0 , 0 ));
		ret.append( boost::lexical_cast<std::string>(OPENBLAS_VERSION));
		return ret;
	}
// 16. metis
	#include <metis.h>
	py::list metisVer() {
		py::list ret;
		ret.append( py::make_tuple(                  METIS_VER_MAJOR   ,                                   METIS_VER_MINOR   ,                                   METIS_VER_SUBMINOR));
		ret.append( boost::lexical_cast<std::string>(METIS_VER_MAJOR)+"."+boost::lexical_cast<std::string>(METIS_VER_MINOR)+"."+boost::lexical_cast<std::string>(METIS_VER_SUBMINOR));
		return ret;
	}
#else
	py::list openblasVer() { return {}; }
	py::list metisVer() { return {}; }
#endif


// 17. mpi
#ifdef YADE_MPI
	#include <mpi.h>
// https://www.open-mpi.org/software/ompi/versions/
	py::list mpiVer() {
		py::list ret;
		ret.append( py::make_tuple                  (OMPI_MAJOR_VERSION   ,                                   OMPI_MINOR_VERSION  ,                                    OMPI_RELEASE_VERSION ));
// I didn't find a way to obtain a version string for mpi, so I construct my own. But if there is some MPI version string, better put it here.
//		ret.append( boost::lexical_cast<std::string>(OMPI_VERSION) );
		ret.append( boost::lexical_cast<std::string>(OMPI_MAJOR_VERSION)+"."+boost::lexical_cast<std::string>(OMPI_MINOR_VERSION)+"."+boost::lexical_cast<std::string>(OMPI_RELEASE_VERSION) );
		return ret;
	}
#else
	py::list mpiVer() { return {}; }
#endif

py::dict getAllVersionsCpp(){
	py::dict ret;
	// I found relevant names with commad:
	// dpkg -L libName-dev |  xargs -I'{}' -P 1 cat {} | grep -i -E "defi.*versio.*"
	ret["gcc"          ] = gccVer();
	ret["clang"        ] = clangVer();
	ret["boost"        ] = boostVer();
	ret["qt"           ] = qtVer();
	ret["freeglut"     ] = freeglutVer();
	ret["gl"           ] = glVer();
	ret["qglviewer"    ] = qglviewerVer();
	ret["python"       ] = pythonVer();
	ret["eigen"        ] = eigenVer();
	ret["sqlite"       ] = sqliteVer();
	ret["vtk"          ] = vtkVer();
	ret["cgal"         ] = cgalVer();
	ret["suitesparse"  ] = suitesparseVer();
	ret["openblas"     ] = openblasVer();
	ret["metis"        ] = metisVer();
	ret["mpi"          ] = mpiVer();
	return ret;
}

BOOST_PYTHON_MODULE(_libVersions){
	YADE_SET_DOCSTRING_OPTS;
/*
 * Maybe exporting them all is not necessary. All data is aggregated inside getAllVersionsCpp() anyway.
 *
	py::def("gccVer"  , gccVer   , "Returns g++   compiler version with which yade was compiled.                                                       \n\n:return: list in format ``[ (major,minor,patch) , \"versionString\" ]``.");
	py::def("clangVer", clangVer , "Returns clang compiler version with which yade was compiled.                                                       \n\n:return: list in format ``[ (major,minor,patch) , \"versionString\" ]``.");
	py::def("boostVer", boostVer , "Returns boost  library version with which yade was compiled.                                                       \n\n:return: list in format ``[ (major,minor,patch) , \"versionString\" ]``.");
	py::def("mpiVer"  , mpiVer   , "Returns MPI    library version with which yade was compiled, as detected from ``#include                 <mpi.h>``.\n\n:return: list in format ``[ (major,minor,patch) , \"versionString\" ]``.");
	py::def("vtkVer"  , vtkVer   , "Returns VTK    library version with which yade was compiled, as detected from ``#include          <vtkVersion.h>``.\n\n:return: list in format ``[ (major,minor,patch) , \"versionString\" ]``.");
	py::def("cgalVer" , cgalVer  , "Returns CGAL   library version with which yade was compiled, as detected from ``#include <CGAL/version_macros.h>``.\n\n:return: list in format ``[ (major,minor,patch) , \"versionString\" ]``.");
*/

// We can use C++ string literal just like """ """ in python to write docstrings (see. https://en.cppreference.com/w/cpp/language/string_literal )
// The """ is a custom delimeter, we could use    R"RAW( instead, or any other delimeter. This decides what will be the termination delimeter.

	py::def("getAllVersionsCpp", getAllVersionsCpp, R"""(
This function returns library versions as discovered by C++ during compilation from all the ``#include`` headers. This can be useful in debugging to detect some library ``.so`` conflicts.

:return: dictionary in folowing format: ``{ "libName" : [ (major, minor, patch) , "versionString" ] }``

As an example the dict below reflects what libraries this documentation was compiled with (here are only those detected by C++):

.. ipython::

   In [1]: from yade.libVersions import *

   In [1]: getAllVersionsCpp()

.. note:: Please add here C++ detection of other libraries when yade starts using them.

	)""");
}

