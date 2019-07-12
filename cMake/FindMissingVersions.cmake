# This is a supplementary file for libVersions module. It forces finding versions of those
# libraries that do not have a detectable version number inside them
# The only way to determine version is by reading the actual source files.
# I do this by checking their md5sum.

##################################################################################
##### Find version of freeglut by reading md5sum of include/GL/freeglut_std.h
##### I have downloaded all available freeglut versions and did on them: 
#####       find -type f -name "freeglut_std.h" -exec md5sum {} \;

find_path(FORCE_FREEGLUT_PATH freeglut_std.h ${GLUT_INCLUDE_DIR}/GL)

execute_process(COMMAND "/usr/bin/md5sum" "${FORCE_FREEGLUT_PATH}/freeglut_std.h"
    RESULT_VARIABLE _FORCE_FREEGLUT_SEARCH_SUCCESS
    OUTPUT_VARIABLE _FORCE_FREEGLUT_VALUES
    ERROR_VARIABLE  _FORCE_FREEGLUT_ERROR_VALUE
    OUTPUT_STRIP_TRAILING_WHITESPACE)

separate_arguments(_FORCE_FREEGLUT_VALUES)

list(GET _FORCE_FREEGLUT_VALUES 0 _FORCE_FREEGLUT_MDSUM)

MESSAGE(STATUS "md5sum of freegult ${FORCE_FREEGLUT_PATH}/freeglut_std.h is: ${_FORCE_FREEGLUT_MDSUM}")

if("${_FORCE_FREEGLUT_MDSUM}" STREQUAL "fce0117bba35ec344ed467bddc4e65e6")
	set(FREEGLUT_VERSION_MAJOR 2)
	set(FREEGLUT_VERSION_MINOR 6)
	set(FREEGLUT_VERSION_PATCH 0)
	set(FREEGLUT_VERSION_STR "2.6.0")
elseif("${_FORCE_FREEGLUT_MDSUM}" STREQUAL "2ee37030c339df044b960e22ae55bf61")
	set(FREEGLUT_VERSION_MAJOR 2)
	set(FREEGLUT_VERSION_MINOR 6)
	set(FREEGLUT_VERSION_PATCH 1)
	set(FREEGLUT_VERSION_STR "2.6.0rc1")
elseif("${_FORCE_FREEGLUT_MDSUM}" STREQUAL "6470390b023f271342287319770e5f51")
	set(FREEGLUT_VERSION_MAJOR 2)
	set(FREEGLUT_VERSION_MINOR 8)
	set(FREEGLUT_VERSION_PATCH 0)
	set(FREEGLUT_VERSION_STR "2.8.0")
elseif("${_FORCE_FREEGLUT_MDSUM}" STREQUAL "791a2febd8584ec530cdd7676191b6d5")
	set(FREEGLUT_VERSION_MAJOR 2)
	set(FREEGLUT_VERSION_MINOR 8)
	set(FREEGLUT_VERSION_PATCH 1)
	set(FREEGLUT_VERSION_STR "2.8.1")
elseif("${_FORCE_FREEGLUT_MDSUM}" STREQUAL "5d350938fc0be29757a26e466fff6414")
	set(FREEGLUT_VERSION_MAJOR 3)
	set(FREEGLUT_VERSION_MINOR 0)
	set(FREEGLUT_VERSION_PATCH 0)
	set(FREEGLUT_VERSION_STR "3.0.0")
else()
	set(FREEGLUT_VERSION_MAJOR -1)
	set(FREEGLUT_VERSION_MINOR -1)
	set(FREEGLUT_VERSION_PATCH -1)
	set(FREEGLUT_VERSION_STR "unknown")
endif()

MESSAGE(STATUS "freegult version is ${FREEGLUT_VERSION_STR}")

## Add below md5sums of other source files if necessary.

