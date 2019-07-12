// 2019 Janek Kozicki

/*
vtk changed their function name in vtk8, this is annoying. And we don't want to litter the code with
#ifdefs everywhere. Better to clean it up with vim comamnds:

:grep -E "InsertNextTupleValue" --include="*" . -R --exclude ChangeLog --exclude tags --exclude CMakeLists.txt
:%s/InsertNextTupleValue(\([^)]\+\))/INSERT_NEXT_TUPLE(\1)/gc

And use a macro in these places:
*/
#pragma once

#ifdef YADE_VTK
	#include <vtkVersion.h>
// fix InsertNextTupleValue → InsertNextTuple name change
	#if VTK_MAJOR_VERSION < 8
		#define INSERT_NEXT_TUPLE(a) InsertNextTupleValue(a)
		#define INSERT_NEXT_TYPED_TUPLE(a) InsertNextTupleValue(a)
	#else
		#define INSERT_NEXT_TUPLE(a) InsertNextTuple(a)
		#define INSERT_NEXT_TYPED_TUPLE(a) InsertNextTypedTuple(a)
	#endif
// (and others in the future)
#endif


/*
# At first I wanted to do this inside cmake, but it turns out that function definitions
# are not supoprted: https://cmake.org/cmake/help/v3.0/prop_dir/COMPILE_DEFINITIONS.html#prop_dir:COMPILE_DEFINITIONS

IF(${VTK_MAJOR_VERSION} LESS 8)
  ADD_DEFINITIONS("-DINSERT_NEXT_TUPLE(a)=InsertNextTupleValue(a)")
ELSE()
  ADD_DEFINITIONS("-DINSERT_NEXT_TUPLE(a)=InsertNextTuple(a)")
ENDIF()
*/

