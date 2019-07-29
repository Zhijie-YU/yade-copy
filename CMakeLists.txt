# The YADE has the following parameters to configure:
#  CMAKE_INSTALL_PREFIX: path, where Yade will be installed (/usr/local by default)
#  LIBRARY_OUTPUT_PATH: path to install libraries (lib by default)
#  DEBUG: compile in debug-mode (OFF by default)
#  ENABLE_LOGGER: use boost::log library for logging (ON by default)
#  MAX_LOG_LEVEL: set maximum level for LOG_* macros compiled with ENABLE_LOGGER. For production build use MAX_LOG_LEVEL<=4, to avoid filter integer comparison on each call to LOG_DEBUG(…), LOG_TRACE(…)
#  CMAKE_VERBOSE_MAKEFILE: output additional information during compiling (OFF by default)
#  SUFFIX: suffix, added after binary-names (version number by default)
#  NOSUFFIX: do not add a suffix after binary-name (OFF by default)
#  YADE_VERSION: explicitely set version number (is defined from git-directory by default)
#  ENABLE_GUI: enable GUI option (ON by default)
#  ENABLE_CGAL: enable CGAL option (ON by default)
#  ENABLE_VTK: enable VTK-export option (ON by default)
#  ENABLE_OPENMP: enable OpenMP-parallelizing option (ON by default)
#  ENABLE_MPI : Enable MPI communications in Yade. Used for Yade-OpenFOAM coupling.
#  ENABLE_GTS: enable GTS-option (ON by default)
#  ENABLE_GL2PS: enable GL2PS-option (ON by default)
#  ENABLE_LINSOLV: enable LINSOLV-option (ON by default)
#  ENABLE_PFVFLOW: enable PFVFLOW-option, FlowEngine (ON by default)
#  ENABLE_TWOPHASEFLOW: enable TWOPHASEFLOW-option, TwoPhaseFlowEngine (ON by default)
#  ENABLE_LBMFLOW: enable LBMFLOW-option, LBM_ENGINE (ON by default)
#  ENABLE_SPH: enable SPH-option, Smoothed Particle Hydrodynamics (OFF by default)
#  ENABLE_LIQMIGRATION: enable LIQMIGRATION-option, see [Mani2013] for details (OFF by default)
#  ENABLE_MASK_ARBITRARY: enable MASK_ARBITRARY option (OFF by default)
#  ENABLE_PROFILING: enable profiling, e.g. shows some more metrics, which can define bottlenecks of the code (OFF by default)
#  ENABLE_POTENTIAL_BLOCKS: enable potential blocks option (OFF by default)
#  ENABLE_POTENTIAL_PARTICLES: enable potential particles option (OFF by default)
#  ENABLE_DEFORM: enable constant volume deformation engine (OFF by default)
#  ENABLE_FEMLIKE: enable meshed solids (FEM-like)
#  ENABLE_OAR: generate a script for oar-based task scheduler (OFF by default)
#  runtimePREFIX: used for packaging, when install directory is not the same is runtime directory (/usr/local by default)
#  CHUNKSIZE: set >1, if you want several sources to be compiled at once. Increases compilation speed and RAM-consumption during it (1 by default)
#  VECTORIZE: enables vectorization and alignment in Eigen3 library, experimental (OFF by default)
#  USE_QT5: use QT5 for GUI (ON by default)
#  CHOLMOD_GPU link Yade to custom SuiteSparse installation and activate GPU accelerated PFV (OFF by default)
#  ENABLE_PYTHON3: enable python 3 compilation instead of python2.7 (OFF by default)
#  PYTHON_VERSION: force python version to the given one, set -1 to automatically use the last version on the system. (-1 by default)

project(Yade C CXX)
cmake_minimum_required(VERSION 2.8.11)

# https://misc.flogisoft.com/bash/tip_colors_and_formatting
# https://stackoverflow.com/questions/18968979/how-to-get-colorized-output-with-cmake , use ${Esc} for printing colors
string(ASCII 27 Esc)
# To have a cmake option with multiple values I used https://cmake.org/pipermail/cmake/2016-October/064342.html
set(MAX_LOG_LEVEL "5" CACHE STRING "set maximum level for LOG_* macros compiled with ENABLE_LOGGER. For production build use MAX_LOG_LEVEL<=4, to avoid filter integer comparison on each call to LOG_DEBUG(…), LOG_TRACE(…)")
set_property(CACHE MAX_LOG_LEVEL PROPERTY STRINGS "0" "1" "2" "3" "4" "5" "6")  # define list of values GUI will offer for the variable


#===========================================================
SET(DEFAULT ON CACHE INTERNAL "Default value for enabled by default options")
SET(LINKLIBS "")
SET(CONFIGURED_FEATS "")
SET(DISABLED_FEATS "")

OPTION(ENABLE_LOGGER "use boost::log library for logging (ON by default)" ON)
OPTION(ENABLE_VTK "Enable VTK" ${DEFAULT})
OPTION(ENABLE_OPENMP "Enable OpenMP" ${DEFAULT})
OPTION(ENABLE_MPI "Enable MPI environement and communications, required for coupling Yade with OpenFOAM  " ON)
OPTION(ENABLE_GTS "Enable GTS" ${DEFAULT})
OPTION(ENABLE_GUI "Enable GUI" ${DEFAULT})
OPTION(ENABLE_CGAL "Enable CGAL" ${DEFAULT})
OPTION(ENABLE_GL2PS "Enable GL2PS" ${DEFAULT})
OPTION(ENABLE_LINSOLV "Enable direct solver for the flow engines (experimental)" ${DEFAULT})
OPTION(ENABLE_PFVFLOW "Enable one-phase flow engines" ${DEFAULT})
OPTION(ENABLE_TWOPHASEFLOW "Enable two-phase flow engines" ${DEFAULT})
OPTION(ENABLE_SPH "Enable SPH" OFF)
OPTION(ENABLE_LBMFLOW "Enable LBM engine (very experimental)" ON)
OPTION(ENABLE_LIQMIGRATION "Enable liquid control (very experimental), see [Mani2013] for details" OFF)
OPTION(ENABLE_MASK_ARBITRARY "Enable arbitrary precision of bitmask variables (only Body::groupMask yet implemented) (experimental). Use -DMASK_ARBITRARY_SIZE=int to set number of used bits (256 by default)" OFF)
OPTION(ENABLE_PROFILING "Enable profiling, e.g. shows some more metrics, which can define bottlenecks of the code (OFF by default)" OFF)
OPTION(ENABLE_POTENTIAL_PARTICLES "Enable potential particles" ON)
OPTION(ENABLE_OAR "Generate script for oar-based task scheduler" OFF)
OPTION(USE_QT5 "USE Qt5 for GUI" ON)
OPTION(ENABLE_POTENTIAL_BLOCKS "Enable PotentialBlocks" ON)
OPTION(ENABLE_DEFORM "Enable Deformation Engine" OFF)
OPTION(ENABLE_FEMLIKE "Enable deformable solids" ON)
OPTION(CHOLMOD_GPU "Enable GPU acceleration flow engine direct solver" OFF)
OPTION(ENABLE_THERMAL "Enable thermal engine (experimental)" ON)
#Set -DPYTHON_VERSION numeric option below:
set(PYTHON_VERSION -1 CACHE STRING "Force python version to the given one. Defaults to -DPYTHON_VERSION=-1 which automatically uses the newest available python version on the system.")
#===========================================================



set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cMake")

INCLUDE(GetVersion)
INCLUDE(GNUInstallDirs)

INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR})

#===========================================================
# HACK!!! If the version of gcc is 4.8 or greater, we add -ftrack-macro-expansion=0
# and -save-temps into compiler to reduce the memory consumption during compilation.
# See http://bugs.debian.org/726009 for more information
# Can be removed later, if gcc fixes its regression
# Taken from http://stackoverflow.com/questions/4058565/check-gcc-minor-in-cmake

# this line was trying to get the g++ version. But it does not work with ccache.
# But we already have CMAKE_CXX_COMPILER_VERSION, so we don't need this line!
#EXECUTE_PROCESS(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
IF (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.8 OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 4.8)
  MESSAGE(STATUS "GCC Version >= 4.8. Adding -ftrack-macro-expansion=0")
  ADD_DEFINITIONS("-ftrack-macro-expansion=0")
ELSE()
  MESSAGE(STATUS "GCC Version < 4.8. will not use -ftrack-macro-expansion=0")
ENDIF()

# -save-temps is not supported with ccache
IF ((CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.8 OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 4.8) AND ( NOT "${CMAKE_CXX_COMPILER_LAUNCHER}" MATCHES ".*ccache" AND NOT "${CMAKE_CXX_COMPILER}" MATCHES ".*ccache.*" AND NOT DISABLE_SAVE_TEMPS))
  MESSAGE(STATUS "GCC Version >= 4.8. Adding -save-temps")
  ADD_DEFINITIONS("-save-temps")
ELSE()
  IF ( "${CMAKE_CXX_COMPILER_LAUNCHER}" MATCHES ".*ccache" OR "${CMAKE_CXX_COMPILER}" MATCHES ".*ccache.*" )
    MESSAGE(STATUS "ccache detected, will not use -save-temps")
# on ubuntu 18.04 and 18.10 when ccache is used we need to fix some bug, see https://gitlab.kitware.com/cmake/cmake/issues/17275
# but we can't do this on ubuntu 16.04 with cmake version 3.5.1, because that produces error: "list does not recognize sub-command FILTER".
# So we must be above version 3.6, because https://cmake.org/cmake/help/v3.6/release/3.6.html?highlight=filter
# Later maybe on some higher cmake version this will become unnecessary.
    IF(CMAKE_VERSION VERSION_GREATER 3.6 OR CMAKE_VERSION VERSION_EQUAL 3.6)
        string (STRIP "${CMAKE_CXX_COMPILER_ARG1}" __CXX_COMPILER_ARG1)
        set(CMAKE_CXX_COMPILER_PREDEFINES_COMMAND "${CMAKE_CXX_COMPILER}" "${__CXX_COMPILER_ARG1}" "-dM" "-E" "-c" "${CMAKE_ROOT}/Modules/CMakeCXXCompilerABI.cpp")
        list(FILTER CMAKE_CXX_COMPILER_PREDEFINES_COMMAND EXCLUDE REGEX "^$")
        unset(__CXX_COMPILER_ARG1)
    ENDIF()
  ELSE()
    MESSAGE(STATUS "GCC Version < 4.8 or DISABLE_SAVE_TEMPS. will not use -save-temps")
  ENDIF()
ENDIF()

#===========================================================

IF ("${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")
  ADD_DEFINITIONS("-ftemplate-depth-512 -fstack-protector ")
ELSE()
  IF (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9 OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 4.9)
    MESSAGE(STATUS "GCC Version >= 4.9. Adding -fstack-protector-strong")
    ADD_DEFINITIONS("-fstack-protector-strong")
  ELSE()
    ADD_DEFINITIONS("-fstack-protector")
  ENDIF()
ENDIF()

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC -O2 --param=ssp-buffer-size=4 -Wformat -Wformat-security -Werror=format-security -Wall -std=c++11")

IF (DEBUG)
  SET(CMAKE_VERBOSE_MAKEFILE 1)
  SET(CMAKE_BUILD_TYPE Debug)
  ADD_DEFINITIONS("-DYADE_DEBUG -g")
ELSE (DEBUG)
  ADD_DEFINITIONS("-DNDEBUG")
ENDIF (DEBUG)

IF(ENABLE_LOGGER)
  MESSAGE(STATUS "${Esc}[36mLog filtering: enabled${Esc}[0m")
  SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} BoostLog")
  ADD_DEFINITIONS("-D YADE_BOOST_LOG")
  ADD_DEFINITIONS("-D BOOST_LOG_DYN_LINK")
  IF (DEBUG)
    MESSAGE(STATUS "Enabling boost::log library and DEBUG=ON. Files will be very large with full debug info inside.")
    IF(${MAX_LOG_LEVEL} LESS 6)
      MESSAGE(STATUS "${Esc}[33mWarning: using DEBUG=ON with MAX_LOG_LEVEL=${MAX_LOG_LEVEL} (less than 6, the maximum log level) rarely makes sense, use cmake option MAX_LOG_LEVEL=6 to fix this.${Esc}[0m")
    ENDIF()
  ELSE (DEBUG)
    MESSAGE(STATUS "Enabling boost::log library and DEBUG=OFF. Logging will work nicely, backtraces will not have debug info, files will be small.")
  ENDIF (DEBUG)
ELSE(ENABLE_LOGGER)
  MESSAGE(STATUS "${Esc}[36mLog filtering: disabled${Esc}[0m")
  IF (DEBUG)
    MESSAGE(STATUS "Disabling boost::log library (only rudimentary LOG_* macros will work) and DEBUG=ON. Files will be very large with full debug info inside.")
  ELSE (DEBUG)
    MESSAGE(STATUS "Disabling boost::log library (only rudimentary LOG_* macros will work, and only up to LOG_WARN) and DEBUG=OFF. Backtraces will not have debug info, files will be small.")
  ENDIF (DEBUG)
ENDIF(ENABLE_LOGGER)

ADD_DEFINITIONS("-DMAX_LOG_LEVEL=${MAX_LOG_LEVEL}")

#==================================
#====== *** PYTHON/BOOST *** ======
#==================================
INCLUDE(YadePythonHelpers)

SET(PY3_VERSIONS 3.8 3.7 3.6 3.5 3.4 3.3 3.2 3.1 3.0) #append newer python versions at the beginning here.
SET(PY2_VERSIONS 2.7 2.6)
IF(${PYTHON_VERSION} EQUAL -1)
	SET(PY_VERSIONS_CHECK "${PY3_VERSIONS};${PY2_VERSIONS}")
ELSEIF(${PYTHON_VERSION} EQUAL 2)
	SET(PY_VERSIONS_CHECK "${PY2_VERSIONS}")
ELSEIF(${PYTHON_VERSION} EQUAL 3)
	SET(PY_VERSIONS_CHECK "${PY3_VERSIONS}")
ELSE()
	SET(PY_VERSIONS_CHECK "${PYTHON_VERSION}")
ENDIF()

MESSAGE("Loop on the following python versions and check available dependencies:${PY_VERSIONS_CHECK}" )
FOREACH(py_version ${PY_VERSIONS_CHECK})
	#Reset variables to be able to use finpythoninterp multiple times
	unset(PYTHONINTERP_FOUND CACHE)
	unset(PYTHON_EXECUTABLE CACHE)
	#Set desired python version
	set(Python_ADDITIONAL_VERSIONS ${py_version})
	#Find python
	FIND_PACKAGE(PythonInterp)
	IF(NOT PythonInterp_FOUND)
		MESSAGE(FATAL_ERROR "No python version could be found in your system.")
	ENDIF()
	#FindPythonInterp will try to fallback in older Python version, we have to check manually if it found the version we wish
	PYTHON_VERSION_MATCHES(${py_version})
	IF(NOT ${PYTHON_VERSION_MATCH})
		CONTINUE()
	ENDIF()
	MESSAGE("Python version ${PYTHON_VERSION_STRING} found, try to import dependencies...")
	FIND_PYTHON_PACKAGES()
	IF(ALL_PYTHON_DEPENDENCIES_FOUND)
		MESSAGE("Found all python dependencies with version ${PYTHON_VERSION_STRING}, will compile yade with that.")
		BREAK()
	ENDIF()
ENDFOREACH()

#After the loop on all python versions, having ALL_PYTHON_DEPENDENCIES_FOUND==FALSE must trigger an error:
IF(NOT ALL_PYTHON_DEPENDENCIES_FOUND)
	MESSAGE(FATAL_ERROR "No python version with all dependencies was found.")
ENDIF()

#======================================
#====== *** END PYTHON/BOOST *** ======
#======================================

#===========================================================
FIND_PACKAGE(Loki REQUIRED)
INCLUDE_DIRECTORIES(${LOKI_INCLUDE_DIR})

FIND_PACKAGE(Eigen3 REQUIRED)
FIND_PACKAGE(BZip2 REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)
#===========================================================

# Use Eigen3 by default
IF (EIGEN3_FOUND)
  INCLUDE_DIRECTORIES(${EIGEN3_INCLUDE_DIR})
  MESSAGE(STATUS "Found Eigen3, version: ${EIGEN3_VERSION}")

  # Minimal required version 3.2.1
  IF ((${EIGEN3_MAJOR_VERSION} LESS 2) OR ((${EIGEN3_MAJOR_VERSION} EQUAL 2) AND (${EIGEN3_MINOR_VERSION} LESS 1)))
    MESSAGE(FATAL_ERROR "Minimal required Eigen3 version is 3.2.1, please update Eigen3!")
  ENDIF ((${EIGEN3_MAJOR_VERSION} LESS 2) OR ((${EIGEN3_MAJOR_VERSION} EQUAL 2) AND (${EIGEN3_MINOR_VERSION} LESS 1)))

  IF (NOT VECTORIZE)
    MESSAGE(STATUS "Disable vectorization")
    ADD_DEFINITIONS("-DEIGEN_DONT_VECTORIZE -DEIGEN_DONT_ALIGN -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT")
  ELSE (NOT VECTORIZE)
    MESSAGE(STATUS "Enable vectorization")
  ENDIF (NOT VECTORIZE)

ENDIF(EIGEN3_FOUND)
#===========================================================
INCLUDE_DIRECTORIES(${BZIP2_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS})
SET(LINKLIBS  "${LINKLIBS};${BZIP2_LIBRARIES};${ZLIB_LIBRARIES};")
#===========================================================
IF((Boost_MAJOR_VERSION EQUAL 1) OR (Boost_MAJOR_VERSION GREATER 1) AND
  ((Boost_MINOR_VERSION EQUAL 53) OR (Boost_MINOR_VERSION GREATER 53)))
  SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} Odeint")
  ADD_DEFINITIONS("-DYADE_ODEINT")
ELSE((Boost_MAJOR_VERSION EQUAL 1) OR (Boost_MAJOR_VERSION GREATER 1) AND
    ((Boost_MINOR_VERSION EQUAL 53) OR (Boost_MINOR_VERSION GREATER 53)))
  SET(DISABLED_FEATS "${DISABLED_FEATS} Odeint")
  MESSAGE(STATUS "Boost Odeint can be enabled, only if Boost>=1.53 is used")
ENDIF((Boost_MAJOR_VERSION EQUAL 1) OR (Boost_MAJOR_VERSION GREATER 1) AND
     ((Boost_MINOR_VERSION EQUAL 53) OR (Boost_MINOR_VERSION GREATER 53)))
#===========================================================
IF(ENABLE_VTK)
  FIND_PACKAGE(VTK COMPONENTS vtkCommonCore vtkIOImage vtkIOXML vtkFiltersCore vtkImagingCore vtkRenderingCore vtkImagingGeneral vtkImagingHybrid)
  IF(VTK_FOUND)
    INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})
    LINK_DIRECTORIES( ${VTK_LIBRARY_DIRS} )
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DYADE_VTK")
    MESSAGE(STATUS "Found VTK")
    SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} VTK")
  ELSE(VTK_FOUND)
    MESSAGE(STATUS "VTK NOT found")
    SET(ENABLE_VTK OFF)
    SET(DISABLED_FEATS "${DISABLED_FEATS} VTK")
  ENDIF(VTK_FOUND)
ELSE(ENABLE_VTK)
  SET(DISABLED_FEATS "${DISABLED_FEATS} VTK")
ENDIF(ENABLE_VTK)
#===========================================================
IF(ENABLE_OPENMP)
  FIND_PACKAGE(OpenMP)
  IF(OPENMP_FOUND)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DYADE_OPENMP ${OpenMP_CXX_FLAGS}")
    MESSAGE(STATUS "Found OpenMP")
    SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} OpenMP")
  ELSE(OPENMP_FOUND)
    MESSAGE(STATUS "OpenMP NOT found")
    SET(ENABLE_OPENMP OFF)
    SET(DISABLED_FEATS "${DISABLED_FEATS} OPENMP")
  ENDIF(OPENMP_FOUND)
ELSE(ENABLE_OPENMP)
  SET(DISABLED_FEATS "${DISABLED_FEATS} OPENMP")
ENDIF(ENABLE_OPENMP)
#===========================================================
IF(ENABLE_GTS)
  FIND_PACKAGE(GTS)
  FIND_PACKAGE(glib2)
  IF(GTS_FOUND AND GLIB2_FOUND)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DYADE_GTS ${CMAKE_GTS_CXX_FLAGS}")
    SET(CMAKE_LD_FLAGS  "${CMAKE_LD_FLAGS} ${GTS_LIBRARIES}")
    INCLUDE_DIRECTORIES(${GTS_INCLUDE_DIR})
    INCLUDE_DIRECTORIES(${GLIB2_INCLUDE_DIRS})
    MESSAGE(STATUS "Found GTS")
    SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} GTS")
  ELSE(GTS_FOUND AND GLIB2_FOUND)
    MESSAGE(STATUS "GTS NOT found")
    SET(DISABLED_FEATS "${DISABLED_FEATS} GTS")
    SET(ENABLE_GTS OFF)
  ENDIF(GTS_FOUND AND GLIB2_FOUND)
ELSE(ENABLE_GTS)
  SET(DISABLED_FEATS "${DISABLED_FEATS} GTS")
ENDIF(ENABLE_GTS)
#===========================================================
IF(ENABLE_GUI)
  FIND_PACKAGE(OpenGL)
  FIND_PACKAGE(GLUT)
  FIND_PACKAGE(glib2)

  IF(USE_QT5)
    MESSAGE(STATUS "USE QT5")
    FIND_PACKAGE(QGLVIEWER-qt5 REQUIRED)
    FIND_PACKAGE(Qt5 CONFIG REQUIRED Widgets Xml OpenGL)
    FIND_PACKAGE(Qt5Widgets)
    IF(Qt5Widgets_FOUND AND OPENGL_FOUND AND GLUT_FOUND AND GLIB2_FOUND AND QGLVIEWER_FOUND)
      SET(GUI_LIBS ${GLUT_LIBRARY} ${QGLVIEWER_LIBRARIES})
      SET(GUI_SRC_LIB "lib/opengl/GLUtils.cpp")
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DYADE_OPENGL -DYADE_QT5")
      SET(Used_QT_VERSION_MAJOR ${QT5_VERSION_MAJOR})
      SET(Used_QT_VERSION_MINOR ${QT5_VERSION_MINOR})
      SET(Used_QT_VERSION_PATCH ${QT5_VERSION_PATCH})
      MESSAGE(STATUS "Found GUI-Qt5-LIBS")
      SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} GUI-Qt5")
    ELSE(Qt5Widgets_FOUND AND OPENGL_FOUND AND GLUT_FOUND AND GLIB2_FOUND AND QGLVIEWER_FOUND)
      MESSAGE(STATUS "GUI-Qt5-LIBS NOT found")
      SET(DISABLED_FEATS "${DISABLED_FEATS} GUI-Qt5")
      SET(ENABLE_GUI OFF)
    ENDIF(Qt5Widgets_FOUND AND OPENGL_FOUND AND GLUT_FOUND AND GLIB2_FOUND AND QGLVIEWER_FOUND)
  ELSE(USE_QT5)   # Use Qt4
    MESSAGE(STATUS "USE QT4")
    FIND_PACKAGE(QGLVIEWER-qt4 REQUIRED)
    FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtOpenGL)
    IF(QT4_FOUND AND OPENGL_FOUND AND GLUT_FOUND AND GLIB2_FOUND AND QGLVIEWER_FOUND)
      SET(GUI_LIBS ${GLUT_LIBRARY} ${OPENGL_LIBRARY} ${QGLVIEWER_LIBRARIES})
      SET(GUI_SRC_LIB "lib/opengl/GLUtils.cpp")
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DYADE_OPENGL -DYADE_QT4")
      SET(Used_QT_VERSION_MAJOR ${QT_VERSION_MAJOR})
      SET(Used_QT_VERSION_MINOR ${QT_VERSION_MINOR})
      SET(Used_QT_VERSION_PATCH ${QT_VERSION_PATCH})
      INCLUDE_DIRECTORIES(${GLIB2_INCLUDE_DIRS})
      INCLUDE_DIRECTORIES(${QT_INCLUDES})

      MESSAGE(STATUS "Found GUI-LIBS")
      SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} GUI")
    ELSE(QT4_FOUND AND OPENGL_FOUND AND GLUT_FOUND AND GLIB2_FOUND AND QGLVIEWER_FOUND)
      MESSAGE(STATUS "GUI-LIBS NOT found")
      SET(DISABLED_FEATS "${DISABLED_FEATS} GUI")
      SET(ENABLE_GUI OFF)
    ENDIF(QT4_FOUND AND OPENGL_FOUND AND GLUT_FOUND AND GLIB2_FOUND AND QGLVIEWER_FOUND)
  ENDIF(USE_QT5)
ELSE(ENABLE_GUI)
  SET(DISABLED_FEATS "${DISABLED_FEATS} GUI")
ENDIF(ENABLE_GUI)
#===========================================================
# This one will cry on attempts to build CGAL-based packages without cgal
IF(NOT ENABLE_CGAL AND (ENABLE_PFVFLOW OR ENABLE_TWOPHASEFLOW))
  MESSAGE(STATUS "PFVFLOW and TWOPHASEFLOW depends on CGAL, and ENABLE_CGAL is disabled (not found or turned OFF)")
ENDIF(NOT ENABLE_CGAL AND (ENABLE_PFVFLOW OR ENABLE_TWOPHASEFLOW))
#===========================================================
IF(ENABLE_CGAL)
  INCLUDE(FindGMP)
  FIND_PACKAGE(CGAL)
  FIND_PACKAGE(GMP)
  IF(CGAL_FOUND AND GMP_FOUND AND (NOT("${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang"))) #Check for clang should be removed, when CGAL will be compilable by clang

    FILE(GLOB CGAL_SRC_LIB "lib/triangulation/*.cpp")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CGAL_DEFINITIONS} -DYADE_CGAL")
    SET(LINKLIBS  "${LINKLIBS};${CGAL_LIBRARIES};${GMP_LIBRARIES};${GMPXX_LIBRARIES};")
    MESSAGE(STATUS "Found CGAL")
    SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} CGAL")

    ADD_DEFINITIONS("-DCGAL_DISABLE_ROUNDING_MATH_CHECK -frounding-math")

    IF(ENABLE_PFVFLOW)
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFLOW_ENGINE")
      SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} PFVFLOW")
    ELSE(ENABLE_PFVFLOW)
      SET(DISABLED_FEATS "${DISABLED_FEATS} PFVFLOW")
    ENDIF(ENABLE_PFVFLOW)

  ELSE(CGAL_FOUND AND GMP_FOUND AND (NOT("${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")))
    MESSAGE(STATUS "CGAL NOT found")
    SET(DISABLED_FEATS "${DISABLED_FEATS} CGAL")
    SET(ENABLE_CGAL OFF)
    IF(ENABLE_PFVFLOW)
      SET(DISABLED_FEATS "${DISABLED_FEATS} PFVFLOW TWOPHASEFLOW")
      MESSAGE(STATUS "CGAL NOT found: PFVFLOW and TWOPHASEFLOW disabled")
    ENDIF(ENABLE_PFVFLOW)

  ENDIF(CGAL_FOUND AND GMP_FOUND AND (NOT("${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")))
ELSE(ENABLE_CGAL)
  SET(DISABLED_FEATS "${DISABLED_FEATS} CGAL")
ENDIF(ENABLE_CGAL)
#===========================================================
IF(ENABLE_LINSOLV)
  FIND_PACKAGE(Cholmod)
  FIND_PACKAGE(OpenBlas)
  FIND_PACKAGE(Metis)

  IF(CHOLMOD_FOUND AND OPENBLAS_FOUND AND METIS_FOUND AND CGAL_FOUND)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CGAL_DEFINITIONS} -DLINSOLV")

    SET(LINKLIBS  "${LINKLIBS};${CHOLMOD_LIBRARIES};${AMD_LIBRARY};${CAMD_LIBRARY};${COLAMD_LIBRARY};${CCOLAMD_LIBRARY};${OPENBLAS_LIBRARY};${METIS_LIBRARY};${SUITESPARSE_LIBRARY}")
    INCLUDE_DIRECTORIES(${METIS_INCLUDE_DIR} ${CHOLMOD_INCLUDE_DIR} ${BLAS_INCLUDE_DIR})
    MESSAGE(STATUS "Found Cholmod in " ${CHOLMOD_LIBRARIES})
    MESSAGE(STATUS "Found OpenBlas in " ${OPENBLAS_LIBRARY})
    MESSAGE(STATUS "Found Metis in " ${METIS_LIBRARY})
    SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} LINSOLV")

    IF(CHOLMOD_GPU)
      FIND_PACKAGE(CuBlas)
      FIND_PACKAGE(Lapack)
      IF(CUBLAS_FOUND AND LAPACK_FOUND)
        ADD_DEFINITIONS("-DPFV_GPU")
        SET(LINKLIBS "${LINKLIBS};${CUBLAS_LIBRARY};${CUDART_LIBRARY};${LAPACK_LIBRARY}")
        MESSAGE(STATUS "Found CuBlas in " ${CUBLAS_LIBRARY})
        MESSAGE(STATUS "Found Lapack in " ${LAPACK_LIBRARY})
        SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} CHOLMOD_GPU")
      ELSE(CUBLAS_FOUND AND LAPACK_FOUND)
        MESSAGE(STATUS "Missing dependency for CHOLMOD_GPU, disabled")
        SET(DISABLED_FEATS "${DISABLED_FEATS} CHOLMOD_GPU")
        SET(CHOLMOD_GPU OFF)
      ENDIF(CUBLAS_FOUND AND LAPACK_FOUND)
    ELSE(CHOLMOD_GPU)
      SET(DISABLED_FEATS "${DISABLED_FEATS} CHOLMOD_GPU")
    ENDIF(CHOLMOD_GPU)

  ELSE(CHOLMOD_FOUND AND OPENBLAS_FOUND AND METIS_FOUND AND CGAL_FOUND)
    MESSAGE(STATUS "Missing dependency for LINSOLV, disabled")
    SET(DISABLED_FEATS "${DISABLED_FEATS} LINSOLV")
    SET(ENABLE_LINSOLV OFF)
  ENDIF(CHOLMOD_FOUND AND OPENBLAS_FOUND AND METIS_FOUND AND CGAL_FOUND)
ELSE(ENABLE_LINSOLV)
  SET(DISABLED_FEATS "${DISABLED_FEATS} LINSOLV")
      IF(ENABLE_TWOPHASEFLOW)
         MESSAGE(STATUS "TWOPHASEFLOW was disabled automatically because LINSOLV is disabled")
         SET(ENABLE_TWOPHASEFLOW OFF)
      ENDIF(ENABLE_TWOPHASEFLOW)
ENDIF(ENABLE_LINSOLV)
#===============================================
IF(ENABLE_MPI)
  FIND_PACKAGE(MPI)
  IF (MPI_FOUND)
    INCLUDE_DIRECTORIES(${MPI_CXX_INCLUDE_PATH})
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MPI_CXX_COMPILE_FLAGS} -DYADE_MPI")
    string (STRIP "${MPI_CXX_LIBRARIES}" MPI_LIBRARIES)
    string (STRIP "${MPI_CXX_LINK_FLAGS}" MPI_LINK_FLAGS)
    SET (LINKLIBS "${LINKLIBS};${MPI_LIBRARIES};${MPI_LINK_FLAGS}")
    MESSAGE(STATUS "MPI found")
    SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} MPI")
    # sometimes mpi.h is not found. cmake should find it, but didn't. I'm not sure why. Here I put all directotries where it can be found.
    # it is an OR operation from ubuntu, debian and opensuse of this: https://packages.debian.org/search?suite=sid&arch=any&mode=path&searchon=contents&keywords=include%2Fmpi.h
#     find_path(MPI_H_INCLUDE_DIR mpi.h /usr/include/openmpi /usr/lib/openmpi /usr/lib/openmpi/include /usr/lib/x86_64-linux-gnu /usr/lib64/mpi/gcc/openmpi 
#     /usr/include/aarch64-linux-gnu /usr/include/arm-linux-gnueabihf /usr/include/i386-kfreebsd-gnu /usr/include/i386-linux-gnu
#     /usr/include/mips64el-linux-gnuabi64 /usr/include/openblas /usr/include/powerpc64-linux-gnu /usr/include/powerpc64le-linux-gnu
#     /usr/include/s390x-linux-gnu /usr/include/sparc64-linux-gnu /usr/include/x86_64-kfreebsd-gnu /usr/include/x86_64-linux-gnu
#     /usr/lib/aarch64-linux-gnu /usr/lib/alpha-linux-gnu /usr/lib/arm-linux-gnueabi /usr/lib/arm-linux-gnueabihf
#     /usr/lib/hppa-linux-gnu /usr/lib/i386-gnu /usr/lib/i386-kfreebsd-gnu /usr/lib/i386-linux-gnu
#     /usr/lib/m68k-linux-gnu /usr/lib/mips-linux-gnu /usr/lib/mips64el-linux-gnuabi64 /usr/lib/mipsel-linux-gnu
#     /usr/lib/powerpc-linux-gnu /usr/lib/powerpc-linux-gnuspe /usr/lib/powerpc64-linux-gnu /usr/lib/powerpc64le-linux-gnu
#     /usr/lib/riscv64-linux-gnu /usr/lib/s390x-linux-gnu /usr/lib/sh4-linux-gnu /usr/lib/sparc64-linux-gnu
#     /usr/lib/x86_64-kfreebsd-gnu /usr/lib/x86_64-linux-gnux32
#     )
#     INCLUDE_DIRECTORIES(${MPI_H_INCLUDE_DIR})
  ELSE(MPI_FOUND)
    MESSAGE(STATUS "MPI NOT FOUND, YADE-OpenFOAM coupling disabled")
    SET (DISABLED_FEATS "${DISABLED_FEATS} MPI")
  ENDIF(MPI_FOUND)
ELSE(ENABLE_MPI)
    SET (DISABLED_FEATS "${DISABLED_FEATS} MPI")
ENDIF(ENABLE_MPI)
#===============================================
IF(ENABLE_TWOPHASEFLOW)
	IF(ENABLE_PFVFLOW)
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTWOPHASEFLOW")
		SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} TWOPHASEFLOW")
	ELSE(ENABLE_PFVFLOW)
		SET(DISABLED_FEATS "${DISABLED_FEATS} TWOPHASEFLOW")
		MESSAGE(STATUS "PFVFLOW disabled, TWOPHASEFLOW will not be compiled")
	ENDIF(ENABLE_PFVFLOW)
ELSE(ENABLE_TWOPHASEFLOW)
	SET(DISABLED_FEATS "${DISABLED_FEATS} TWOPHASEFLOW")
ENDIF(ENABLE_TWOPHASEFLOW)
#===============================================
IF(ENABLE_SPH)
  ADD_DEFINITIONS("-DYADE_SPH")
  SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} SPH")
ELSE(ENABLE_SPH)
  SET(DISABLED_FEATS "${DISABLED_FEATS} SPH")
ENDIF(ENABLE_SPH)
#===============================================
IF(ENABLE_DEFORM)
  ADD_DEFINITIONS("-DYADE_DEFORM")
  SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} DEFORM")
ELSE(ENABLE_DEFORM)
  SET(DISABLED_FEATS "${DISABLED_FEATS} DEFORM")
ENDIF(ENABLE_DEFORM)
#===============================================
IF(ENABLE_FEMLIKE)
  ADD_DEFINITIONS("-DYADE_FEM")
  SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} FEMLIKE")
ELSE(ENABLE_FEMLIKE)
  SET(DISABLED_FEATS "${DISABLED_FEATS} FEMLIKE")
ENDIF(ENABLE_FEMLIKE)
#===============================================
IF(ENABLE_LIQMIGRATION)
  ADD_DEFINITIONS("-DYADE_LIQMIGRATION")
  SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} LIQMIGRATION")
ELSE(ENABLE_LIQMIGRATION)
  SET(DISABLED_FEATS "${DISABLED_FEATS} LIQMIGRATION")
ENDIF(ENABLE_LIQMIGRATION)
#===============================================
IF(ENABLE_GL2PS)
  FIND_PACKAGE(GL2PS)
  IF(GL2PS_FOUND)
    INCLUDE_DIRECTORIES(${GL2PS_INCLUDE_DIR})
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DYADE_GL2PS")
    SET(LINKLIBS  "${LINKLIBS};${GL2PS_LIBRARIES};")
    MESSAGE(STATUS "Found GL2PS")
    SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} GL2PS")
  ELSE(GL2PS_FOUND)
    MESSAGE(STATUS "GL2PS NOT found")
    SET(DISABLED_FEATS "${DISABLED_FEATS} GL2PS")
    SET(ENABLE_GL2PS OFF)
  ENDIF(GL2PS_FOUND)
ELSE(ENABLE_GL2PS)
  SET(DISABLED_FEATS "${DISABLED_FEATS} GL2PS")
ENDIF(ENABLE_GL2PS)

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

#===========================================================
IF(ENABLE_LBMFLOW)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLBM_ENGINE")
  SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} LBMFLOW")
  MESSAGE("LBMFLOW is still experimental, building and running LBM engine are at your own risk!")
ELSE(ENABLE_LBMFLOW)
  SET(DISABLED_FEATS "${DISABLED_FEATS} LBMFLOW")
ENDIF(ENABLE_LBMFLOW)
#===============================================
IF(ENABLE_MASK_ARBITRARY)
  ADD_DEFINITIONS("-DYADE_MASK_ARBITRARY")
  SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} MASK_ARBITRARY")
  IF(NOT MASK_ARBITRARY_SIZE)
    SET(MASK_ARBITRARY_SIZE "256")
  ENDIF(NOT MASK_ARBITRARY_SIZE)
  ADD_DEFINITIONS(-DYADE_MASK_ARBITRARY_SIZE=${MASK_ARBITRARY_SIZE})
  MESSAGE("MASK_ARBITRARY_SIZE = ${MASK_ARBITRARY_SIZE}")
ELSE(ENABLE_MASK_ARBITRARY)
  SET(DISABLED_FEATS "${DISABLED_FEATS} MASK_ARBITRARY")
ENDIF(ENABLE_MASK_ARBITRARY)

#===========================================================

IF(ENABLE_THERMAL)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTHERMAL")
  SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} THERMAL")
ELSE(ENABLE_THERMAL)
  SET(DISABLED_FEATS "${DISABLED_FEATS} THERMAL")
ENDIF(ENABLE_THERMAL)

IF(ENABLE_PROFILING)
  SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} PROFILING")
  ADD_DEFINITIONS("-DUSE_TIMING_DELTAS -DISC_TIMING")
ELSE(ENABLE_PROFILING)
  SET(DISABLED_FEATS "${DISABLED_FEATS} PROFILING")
ENDIF(ENABLE_PROFILING)

#===========================================================
IF(ENABLE_POTENTIAL_PARTICLES)
  FIND_PACKAGE(OpenBlas REQUIRED)
  FIND_PACKAGE(LAPACK REQUIRED)
  IF(OPENBLAS_FOUND AND LAPACK_FOUND)
    ADD_DEFINITIONS("-DYADE_POTENTIAL_PARTICLES")
    SET(LINKLIBS  "${LINKLIBS};${OPENBLAS_LIBRARY};${LAPACK_LIBRARY}")
    MESSAGE(STATUS "Found OpenBlas")
    MESSAGE(STATUS "Found Lapack")
    INCLUDE_DIRECTORIES(${BLAS_INCLUDE_DIR})
    SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} PotentialParticles")
  ELSE(OPENBLAS_FOUND AND LAPACK_FOUND)
    MESSAGE(STATUS "Missing dependency for PotentialParticles, disabled")
    SET(DISABLED_FEATS "${DISABLED_FEATS} PotentialParticles")
    SET(ENABLE_POTENTIAL_PARTICLES OFF)
  ENDIF(OPENBLAS_FOUND AND LAPACK_FOUND)
ELSE(ENABLE_POTENTIAL_PARTICLES)
  SET(DISABLED_FEATS "${DISABLED_FEATS} PotentialParticles")
ENDIF(ENABLE_POTENTIAL_PARTICLES)
#===========================================================


#===========================================================
IF(ENABLE_POTENTIAL_BLOCKS)
  INCLUDE(FindCLP)
  FIND_PACKAGE(OpenBlas REQUIRED)
  FIND_PACKAGE(LAPACK REQUIRED)
  IF(CLP_FOUND AND OPENBLAS_FOUND AND LAPACK_FOUND)
    ADD_DEFINITIONS("-DYADE_POTENTIAL_BLOCKS")
    INCLUDE_DIRECTORIES(${CLP_INCLUDE_DIR} ${CLP2_INCLUDE_DIR} )
    SET(LINKLIBS  "${LINKLIBS};${CLP_LIBRARY};${CLP2_LIBRARY};${CLP3_LIBRARY};${OPENBLAS_LIBRARY};${LAPACK_LIBRARY}")
    MESSAGE(STATUS "Found CLP")
    INCLUDE_DIRECTORIES(${BLAS_INCLUDE_DIR})
    SET(CONFIGURED_FEATS "${CONFIGURED_FEATS} PotentialBlocks")
  ELSE(CLP_FOUND AND OPENBLAS_FOUND AND LAPACK_FOUND)
    MESSAGE(STATUS "CLP NOT found")
    SET(DISABLED_FEATS "${DISABLED_FEATS} PotentialBlocks")
    SET(ENABLE_POTENTIAL_BLOCKS OFF)
  ENDIF(CLP_FOUND AND OPENBLAS_FOUND AND LAPACK_FOUND)
ELSE(ENABLE_POTENTIAL_BLOCKS)
  SET(DISABLED_FEATS "${DISABLED_FEATS} PotentialBlocks")
ENDIF(ENABLE_POTENTIAL_BLOCKS)
#===========================================================


#===========================================================
#= Forced detection of library versions for libVersions ====
#===========================================================
INCLUDE(FindMissingVersions)
#===========================================================



IF (INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX ${INSTALL_PREFIX})
  MESSAGE(WARNING "Use CMAKE_INSTALL_PREFIX option instead of INSTALL_PREFIX! It will be removed soon.")
ENDIF (INSTALL_PREFIX)

IF (CMAKE_INSTALL_PREFIX)
  MESSAGE("Yade will be installed to ${CMAKE_INSTALL_PREFIX}")
ELSE (CMAKE_INSTALL_PREFIX)
  MESSAGE("Yade will be installed to default path ${CMAKE_INSTALL_PREFIX}, if you want to override it use -DCMAKE_INSTALL_PREFIX option.")
ENDIF (CMAKE_INSTALL_PREFIX)

IF (NOT SUFFIX)
  SET (SUFFIX "-${YADE_VERSION}")
ENDIF (NOT SUFFIX)

IF(NOSUFFIX)   #For packaging
  SET (SUFFIX "")
ENDIF(NOSUFFIX)   #For packaging

IF(NOT LIBRARY_OUTPUT_PATH)   #For packaging
  SET (LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_LIBDIR})
ENDIF(NOT LIBRARY_OUTPUT_PATH)   #For packaging

IF (NOT runtimePREFIX)
  SET (runtimePREFIX ${CMAKE_INSTALL_PREFIX})
ENDIF (NOT runtimePREFIX)

MESSAGE (STATUS "Suffix is set to " ${SUFFIX})
MESSAGE (STATUS "LIBRARY_OUTPUT_PATH is set to " ${LIBRARY_OUTPUT_PATH})
MESSAGE (STATUS "runtimePREFIX is set to " ${runtimePREFIX})
#===========================================================

SET(YADE_LIB_PATH ${CMAKE_INSTALL_PREFIX}/${LIBRARY_OUTPUT_PATH}/yade${SUFFIX})
SET(YADE_EXEC_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
SET(YADE_PY_PATH ${YADE_LIB_PATH}/py)
SET(YADE_DOC_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/doc/yade${SUFFIX})
SET(YADE_MAN_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_MANDIR})


SET(CMAKE_SKIP_BUILD_RPATH FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${YADE_LIB_PATH};${YADE_PY_PATH};${YADE_PY_PATH}/yade/;${YADE_PY_PATH}/yade/qt;${YADE_PY_PATH}/gts")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

#===========================================================
IF(ENABLE_GUI)
  ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/gui")
ENDIF(ENABLE_GUI)
ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/py")
#===========================================================

FILE(GLOB SRC_CORE "core/*.cpp")
FILE(GLOB_RECURSE SRC_PKG  "pkg/*.cpp")
FILE(GLOB SRC_LIB  "lib/*.cpp")

SET(SRC_LIB "${SRC_LIB};lib/base/Math.cpp;lib/factory/ClassFactory.cpp;lib/factory/DynLibManager.cpp;lib/base/Logging.cpp")
SET(SRC_LIB "${SRC_LIB};lib/serialization/Serializable.cpp;lib/pyutil/gil.cpp;core/main/pyboot.cpp;${GUI_SRC_LIB};${CGAL_SRC_LIB}")
IF(ENABLE_POTENTIAL_PARTICLES OR ENABLE_POTENTIAL_BLOCKS)
  SET(SRC_LIB "${SRC_LIB};lib/computational-geometry/MarchingCube.cpp")
ENDIF(ENABLE_POTENTIAL_PARTICLES OR ENABLE_POTENTIAL_BLOCKS)

#===========================================================

IF (CHUNKSIZE)
  INCLUDE(CombineSources)
  COMBINE_SOURCES(${CMAKE_BINARY_DIR}/core "${SRC_CORE}" ${CHUNKSIZE})
  FILE(GLOB SRC_CORE_COMBINED "${CMAKE_BINARY_DIR}/core.*.cpp")
  COMBINE_SOURCES(${CMAKE_BINARY_DIR}/pkg "${SRC_PKG}" ${CHUNKSIZE})
  FILE(GLOB SRC_PKG_COMBINED "${CMAKE_BINARY_DIR}/pkg.*.cpp")
  COMBINE_SOURCES(${CMAKE_BINARY_DIR}/lib "${SRC_LIB}" ${CHUNKSIZE})
  FILE(GLOB SRC_LIB_COMBINED "${CMAKE_BINARY_DIR}/lib.*.cpp")
  ADD_LIBRARY(yade SHARED ${SRC_LIB_COMBINED} ${SRC_CORE_COMBINED} ${SRC_PKG_COMBINED})
ELSE (CHUNKSIZE)
  ADD_LIBRARY(yade SHARED ${SRC_CORE} ${SRC_PKG} ${SRC_LIB})
ENDIF (CHUNKSIZE)

#===========================================================

ADD_LIBRARY(boot SHARED ${CMAKE_CURRENT_SOURCE_DIR}/core/main/pyboot.cpp)
SET_TARGET_PROPERTIES(boot PROPERTIES PREFIX "" LINK_FLAGS "-Wl,--as-needed" )
TARGET_LINK_LIBRARIES(yade ${Boost_LIBRARIES} ${PYTHON_LIBRARIES} ${LINKLIBS} -lrt)
SET_TARGET_PROPERTIES(yade  PROPERTIES LINK_FLAGS "-Wl,--as-needed" )
TARGET_LINK_LIBRARIES(boot yade)

IF(ENABLE_VTK)
  IF(${VTK_MAJOR_VERSION} GREATER 5)
    TARGET_LINK_LIBRARIES(yade ${VTK_LIBRARIES})
    ADD_DEFINITIONS("-DYADE_VTK6")
  IF(${VTK_MAJOR_VERSION} GREATER 7)
   ADD_DEFINITIONS("-DYADE_VTK8")
   MESSAGE(STATUS "VTK version >7 is found")
  ELSE(${VTK_MAJOR_VERSION} GREATER 7)
   MESSAGE(STATUS "VTK version >5 and <8 is found")
  ENDIF(${VTK_MAJOR_VERSION} GREATER 7)
   ELSE(${VTK_MAJOR_VERSION} GREATER 5)
    TARGET_LINK_LIBRARIES(yade vtkHybrid)
  ENDIF(${VTK_MAJOR_VERSION} GREATER 5)
ENDIF(ENABLE_VTK)

IF(ENABLE_GUI)
  TARGET_LINK_LIBRARIES(yade _GLViewer ${GUI_LIBS})
ENDIF(ENABLE_GUI)

#====================================
#Back compatibility with scons
SET (realVersion ${YADE_VERSION})
SET (version ${YADE_VERSION})
SET (pyExecutable ${PYTHON_EXECUTABLE})
SET (profile "default")
SET (sourceRoot "${CMAKE_CURRENT_SOURCE_DIR}")
#====================================

CONFIGURE_FILE(core/main/yade-batch.in "${CMAKE_BINARY_DIR}/bins/yade${SUFFIX}-batch")
IF(ENABLE_OAR)
  CONFIGURE_FILE(core/main/yade-oar.in "${CMAKE_BINARY_DIR}/bins/yade${SUFFIX}-oar")
ENDIF(ENABLE_OAR)
CONFIGURE_FILE(core/main/main.py.in "${CMAKE_BINARY_DIR}/bins/yade${SUFFIX}")
CONFIGURE_FILE(py/config.py.in "${CMAKE_BINARY_DIR}/config.py")
CONFIGURE_FILE(py/libVersions.py.in "${CMAKE_BINARY_DIR}/libVersions.py")
CONFIGURE_FILE(py/__init__.py.in "${CMAKE_BINARY_DIR}/__init__.py")

#===========================================================
# Create header files for PFV from FlowEngine.hpp.in-template.
# All @TEMPLATE_FLOW_NAME@ are replacing by a given names

SET (TEMPLATE_FLOW_NAMES DFNFlowEngineT DummyFlowEngineT FlowEngineT FlowEngine_PeriodicInfo SoluteFlowEngineT UnsaturatedEngineT TwoPhaseFlowEngineT)
FOREACH(TF ${TEMPLATE_FLOW_NAMES})
  SET (TEMPLATE_FLOW_NAME ${TF})
  CONFIGURE_FILE(pkg/pfv/FlowEngine.hpp.in "${CMAKE_BINARY_DIR}/pkg/pfv/FlowEngine_${TF}.hpp" @ONLY)
  CONFIGURE_FILE(pkg/pfv/FlowEngine.ipp.in "${CMAKE_BINARY_DIR}/pkg/pfv/FlowEngine_${TF}.ipp" @ONLY)
ENDFOREACH(TF)
INCLUDE_DIRECTORIES("${CMAKE_BINARY_DIR}/pkg/pfv/")
#===========================================================

INSTALL(PROGRAMS "${CMAKE_BINARY_DIR}/bins/yade${SUFFIX}-batch" DESTINATION ${YADE_EXEC_PATH}/)
IF(ENABLE_OAR)
  INSTALL(PROGRAMS "${CMAKE_BINARY_DIR}/bins/yade${SUFFIX}-oar" DESTINATION ${YADE_EXEC_PATH}/)
ENDIF(ENABLE_OAR)
INSTALL(PROGRAMS "${CMAKE_BINARY_DIR}/bins/yade${SUFFIX}" DESTINATION ${YADE_EXEC_PATH}/)
INSTALL(FILES "${CMAKE_BINARY_DIR}/config.py" DESTINATION ${YADE_PY_PATH}/yade/)
INSTALL(FILES "${CMAKE_BINARY_DIR}/libVersions.py" DESTINATION ${YADE_PY_PATH}/yade/)
INSTALL(FILES "${CMAKE_BINARY_DIR}/__init__.py" DESTINATION ${YADE_PY_PATH}/yade/)
FILE(GLOB filesPYChecks "${CMAKE_CURRENT_SOURCE_DIR}/scripts/checks-and-tests/checks/*.py")
INSTALL(FILES ${filesPYChecks} DESTINATION ${YADE_PY_PATH}/yade/tests/checks)
FILE(GLOB filesPYChecksData "${CMAKE_CURRENT_SOURCE_DIR}/scripts/checks-and-tests/checks/data/*")
INSTALL(FILES ${filesPYChecksData} DESTINATION ${YADE_PY_PATH}/yade/tests/checks/data)
INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/doc/yade-logo-note.png" DESTINATION "${YADE_DOC_PATH}/img")

INSTALL(TARGETS boot DESTINATION "${YADE_PY_PATH}/yade/")
INSTALL(TARGETS yade DESTINATION ${YADE_LIB_PATH})

#===========================================================
MESSAGE(STATUS "===========================================================")
MESSAGE(STATUS "Yade configured with following features:${CONFIGURED_FEATS}")
MESSAGE(STATUS "Disabled features:${DISABLED_FEATS}")
IF(ENABLE_LOGGER)
  MESSAGE(STATUS "${Esc}[36mUsing MAX_LOG_LEVEL=${MAX_LOG_LEVEL}, ENABLE_LOGGER=ON${Esc}[0m")
ELSE(ENABLE_LOGGER)
  MESSAGE(STATUS "${Esc}[36mUsing MAX_LOG_LEVEL=${MAX_LOG_LEVEL}, ENABLE_LOGGER=OFF${Esc}[0m")
ENDIF(ENABLE_LOGGER)
IF (DEBUG)
  MESSAGE(STATUS "Debug build")
  SET (debugbuild " (debug build)")
ELSE (DEBUG)
  MESSAGE(STATUS "Optimized build")
ENDIF (DEBUG)
IF (CHUNKSIZE)
  MESSAGE(STATUS "CHUNKSIZE is set to " ${CHUNKSIZE})
ENDIF (CHUNKSIZE)
MESSAGE(STATUS "===========================================================")
#===========================================================
#Building doc
ADD_CUSTOM_TARGET(doc)
FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/doc)
FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/doc/sphinx)
ADD_CUSTOM_COMMAND(
                  TARGET doc PRE_BUILD
                  COMMAND rm -rf ${CMAKE_BINARY_DIR}/doc/sphinx/_build
                  COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/doc/* ${CMAKE_BINARY_DIR}/doc
                  COMMAND PYTHONPATH=${CMAKE_BINARY_DIR}/doc/sphinx ${YADE_EXEC_PATH}/yade${SUFFIX} -x yadeSphinx.py html
                  COMMAND PYTHONPATH=${CMAKE_BINARY_DIR}/doc/sphinx ${YADE_EXEC_PATH}/yade${SUFFIX} -x yadeSphinx.py latex
                  COMMAND PYTHONPATH=${CMAKE_BINARY_DIR}/doc/sphinx ${YADE_EXEC_PATH}/yade${SUFFIX} -x yadeSphinx.py epub
                  COMMAND PYTHONPATH=${CMAKE_BINARY_DIR}/doc/sphinx ${YADE_EXEC_PATH}/yade${SUFFIX} -x yadeSphinx.py workarounds
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc/sphinx
                  DEPENDS ${YADE_EXEC_PATH}/yade${SUFFIX}
                  )
ADD_CUSTOM_COMMAND(
                  TARGET doc POST_BUILD
                  COMMAND xelatex Yade.tex
                  COMMAND xelatex Yade.tex
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc/sphinx/_build/latex
                  DEPENDS ${YADE_EXEC_PATH}/yade${SUFFIX} ${CMAKE_BINARY_DIR}/doc/sphinx/_build/latex/Yade.tex
                  )
ADD_CUSTOM_COMMAND(
                  TARGET doc POST_BUILD
                  COMMAND rm -rf ${YADE_DOC_PATH}/html
                  COMMAND mv ${CMAKE_BINARY_DIR}/doc/sphinx/_build/html ${YADE_DOC_PATH}/html
                  COMMAND mv ${CMAKE_BINARY_DIR}/doc/sphinx/_build/latex/Yade.pdf ${YADE_DOC_PATH}/
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc/sphinx/_build/latex
                  DEPENDS ${YADE_EXEC_PATH}/yade${SUFFIX} ${CMAKE_BINARY_DIR}/doc/sphinx/_build/latex/Yade.tex
                  )
ADD_CUSTOM_COMMAND(
                  TARGET doc POST_BUILD
                  COMMAND mv ${CMAKE_BINARY_DIR}/doc/sphinx/_build/epub/Yade.epub ${YADE_DOC_PATH}/ || true
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc/sphinx/_build/latex
                  DEPENDS ${YADE_EXEC_PATH}/yade${SUFFIX} ${CMAKE_BINARY_DIR}/doc/sphinx/_build/latex/Yade.tex
                  )
#===========================================================
#Building manpage
ADD_CUSTOM_TARGET(manpage)
ADD_CUSTOM_COMMAND(
                  TARGET manpage POST_BUILD
                  COMMAND help2man ${YADE_EXEC_PATH}/yade${SUFFIX} > yade${SUFFIX}.1
                  COMMAND help2man ${YADE_EXEC_PATH}/yade${SUFFIX}-batch > yade${SUFFIX}-batch.1
                  COMMAND mkdir -p ${YADE_MAN_PATH}
                  COMMAND mv *.1 ${YADE_MAN_PATH}
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                  DEPENDS ${YADE_EXEC_PATH}/yade${SUFFIX} /usr/bin/help2man
                  )
#===========================================================
#Execute standard checks
ADD_CUSTOM_TARGET(check)
ADD_CUSTOM_COMMAND(
                  TARGET check POST_BUILD
                  COMMAND ${YADE_EXEC_PATH}/yade${SUFFIX} --test
                  COMMAND ${YADE_EXEC_PATH}/yade${SUFFIX} --check
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                  DEPENDS ${YADE_EXEC_PATH}/yade${SUFFIX}
                  )
#===========================================================
