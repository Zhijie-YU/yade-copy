##########################################################################
#  Copyright (C) 2019 by Kneib Francois                                  #
#                                                                        #
#  This program is free software; it is licensed under the terms of the  #
#  GNU General Public License v2 or later. See file LICENSE for details. #
##########################################################################


# http://www.cmake.org/pipermail/cmake/2011-January/041666.html
#
# - Find Python Module
FUNCTION(FIND_PYTHON_MODULE module)
  #Reset result value as this function can be called multiple times while testing different python versions:
  UNSET(PY_${module} CACHE)
  STRING(TOUPPER ${module} module_upper)
  
  IF(ARGC GREATER 1 AND ARGV1 STREQUAL "REQUIRED")
    SET(${module}_FIND_REQUIRED TRUE)
  ENDIF(ARGC GREATER 1 AND ARGV1 STREQUAL "REQUIRED")
  
  EXECUTE_PROCESS(COMMAND "${PYTHON_EXECUTABLE}" "-c" 
    "from __future__ import print_function ; import re, ${module} ; print (re.compile('/__init__.py.*').sub('',${module}.__file__))" #NOTE: from __future__ imports MUST be at the beginning of command.
    RESULT_VARIABLE _${module}_status 
    OUTPUT_VARIABLE _${module}_location
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    
    IF(_${module}_status MATCHES 0)
      SET(PY_${module} ${_${module}_location} CACHE STRING "Location of Python module ${module}")
    ENDIF(_${module}_status MATCHES 0)
    
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(${module} DEFAULT_MSG PY_${module})
  #We are in a function, make the result available in the parent scope which is the main CMakeLists.txt:
  IF(${module}_FOUND)
    SET(${module}_FOUND ${${module}_FOUND} PARENT_SCOPE)
  ELSE()
    UNSET(${module}_FOUND PARENT_SCOPE)
  ENDIF()
ENDFUNCTION(FIND_PYTHON_MODULE)


# Find PythonLibs, all Python packages needed by yade, and libboost-python. Must be used after a call to FIND_PACKAGE(PythonInterp)
FUNCTION(FIND_PYTHON_PACKAGES)
	SET(ALL_PYTHON_DEPENDENCIES_FOUND FALSE PARENT_SCOPE)
	SET(fail_message "Failed to import dependencies for Python version ${PYTHON_VERSION_STRING}. NOT FOUND:")

	UNSET(PYTHON_LIBRARY CACHE)
	UNSET(PYTHON_INCLUDE_DIR CACHE)
	FIND_PACKAGE(PythonLibs QUIET)
	IF(NOT PYTHONLIBS_FOUND)
		MESSAGE(${fail_message} PythonLibs)
		RETURN()
	ENDIF()

	# BEGIN find Boost for py_version
	IF ( NOT LocalBoost )
		SET(LocalBoost "1.47.0") # Minimal required Boost version
	ENDIF ( NOT LocalBoost )
	IF ( ${PYTHON_VERSION_MAJOR} EQUAL 2 ) # Python 2
		IF(ENABLE_LOGGER)
			FIND_PACKAGE(Boost ${LocalBoost}  QUIET COMPONENTS python thread filesystem iostreams regex serialization system date_time log)
		ELSE(ENABLE_LOGGER)
			FIND_PACKAGE(Boost ${LocalBoost}  QUIET COMPONENTS python thread filesystem iostreams regex serialization system date_time)
		ENDIF(ENABLE_LOGGER)
	ELSE() # Python 3: next loop is due to libboost-pythonXXX naming mismatch between ubuntu versions and debian versions, so try two possibilities that cover all distros.
		FOREACH(PYTHON_PREFIX python-py python3-py)
			IF(ENABLE_LOGGER)
				FIND_PACKAGE(Boost ${LocalBoost}  QUIET COMPONENTS ${PYTHON_PREFIX}${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR} thread filesystem iostreams regex serialization system date_time log)
			ELSE(ENABLE_LOGGER)
				FIND_PACKAGE(Boost ${LocalBoost}  QUIET COMPONENTS ${PYTHON_PREFIX}${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR} thread filesystem iostreams regex serialization system date_time)
			ENDIF(ENABLE_LOGGER)
			IF(Boost_FOUND)
				BREAK()
			ENDIF()
		ENDFOREACH()
		IF(NOT Boost_FOUND) # for opensuze
			IF(ENABLE_LOGGER)
				FIND_PACKAGE(Boost ${LocalBoost}  QUIET COMPONENTS python-py${PYTHON_VERSION_MAJOR} thread filesystem iostreams regex serialization system date_time log)
			ELSE(ENABLE_LOGGER)
				FIND_PACKAGE(Boost ${LocalBoost}  QUIET COMPONENTS python-py${PYTHON_VERSION_MAJOR} thread filesystem iostreams regex serialization system date_time)
			ENDIF(ENABLE_LOGGER)
		ENDIF()
	ENDIF()
	IF(NOT Boost_FOUND) #as we try multiple python prefixes we have to handle manually the required behavior: fail if we didn't found boost
		MESSAGE(${fail_message} libboost-python)
		RETURN()
	ENDIF()
	# END find Boost for py_version
	
	# find Python modules. WARNING: each FindXXX.cmake MUST unset XXX_FOUND or set XXX_FOUND to FALSE [PARENT_SCOPE] if the module was not found.
	FIND_PACKAGE(NumPy QUIET)
	IF(NOT NUMPY_FOUND)
		MESSAGE(${fail_message} numpy)
		RETURN()
	ENDIF()
	FOREACH(PYTHON_MODULE IPython matplotlib pygraphviz Xlib minieigen future past)
		FIND_PYTHON_MODULE(${PYTHON_MODULE} QUIET)
		IF( NOT ${PYTHON_MODULE}_FOUND )
			MESSAGE(${fail_message} ${PYTHON_MODULE})
			RETURN()
		ENDIF()
	ENDFOREACH()
	IF(${PYTHON_VERSION_MAJOR} EQUAL 2)
		FIND_PYTHON_MODULE(Tkinter QUIET)
		IF(NOT Tkinter_FOUND)
			MESSAGE(${fail_message} Tkinter)
			RETURN()
		ENDIF()
	ELSE()
		FIND_PYTHON_MODULE(tkinter QUIET)
		IF(NOT tkinter_FOUND)
			MESSAGE(${fail_message} tkinter)
			RETURN()
		ENDIF()
	ENDIF()
	
	# NOTE: If we are here, we found a suitable Python version with all packages needed.
	SET(ALL_PYTHON_DEPENDENCIES_FOUND TRUE PARENT_SCOPE)
	#Export findpythonlibs vars to global parent scope:
	FOREACH(pythonlibs_var PYTHONLIBS_FOUND PYTHON_LIBRARIES PYTHON_INCLUDE_PATH PYTHON_INCLUDE_DIRS PYTHONLIBS_VERSION_STRING)
		SET(${pythonlibs_var} ${${pythonlibs_var}} PARENT_SCOPE)
	ENDFOREACH()
	INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
	INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})
	INCLUDE_DIRECTORIES(${NUMPY_INCLUDE_DIRS})
	INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
	#Export findboost vars to global parent scope:
	FOREACH(boost_var boost_FOUND Boost_INCLUDE_DIRS Boost_LIBRARY_DIRS Boost_LIBRARIES Boost_<C>_FOUND Boost_<C>_LIBRARY Boost_VERSION Boost_LIB_VERSION Boost_MAJOR_VERSION Boost_MINOR_VERSION Boost_SUBMINOR_VERSION)
		SET(${boost_var} ${${boost_var}} PARENT_SCOPE)
	ENDFOREACH()
	# for checking purpose
	MESSAGE("--   Boost_VERSION: " ${Boost_VERSION})
	MESSAGE("--   Boost_LIB_VERSION: " ${Boost_LIB_VERSION})
	MESSAGE("--   Boost_INCLUDE_DIRS: " ${Boost_INCLUDE_DIRS})
	MESSAGE("--   Boost_LIBRARIES: " ${Boost_LIBRARIES})
	ADD_DEFINITIONS(-DBOOST_MATH_DISABLE_FLOAT128=1)
	
ENDFUNCTION(FIND_PYTHON_PACKAGES)

# Did findpythoninterp found the python version we want ? Output in PYTHON_VERSION_MATCH.
FUNCTION(PYTHON_VERSION_MATCHES version_number)
	SET(PYTHON_VERSION_MATCH FALSE PARENT_SCOPE)
	STRING(REGEX MATCHALL "[^\\.]" numbers_list ${version_number})
	list(LENGTH numbers_list numbers_list_len)
	
	LIST(GET numbers_list 0 major)
	IF(NOT (${PYTHON_VERSION_MAJOR} EQUAL ${major}))
		RETURN()
	ENDIF()
	
	IF(${numbers_list_len} GREATER 1)
		LIST(GET numbers_list 1 minor)
		IF(NOT(${PYTHON_VERSION_MINOR} EQUAL ${minor}))
			RETURN()
		ENDIF()
	ENDIF()
	#if we are here we match major and minor
	SET(PYTHON_VERSION_MATCH TRUE PARENT_SCOPE)
ENDFUNCTION(PYTHON_VERSION_MATCHES version_number)
