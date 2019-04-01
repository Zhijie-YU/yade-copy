#include<lib/pyutil/gil.hpp>
void pyRunString(const std::string& cmd){
	// FIXME: when a new function is declared inside ipython session an extra python command must be called: globals().update(locals())
	//        https://stackoverflow.com/questions/43956636/internal-function-call-not-working-in-ipython
	//        calling it always here, before each call to cmd.c_str() is worth discussing.
	//        For now I will just add a note about that in documentation: doc/sphinx/tutorial-data-mining.rst
	gilLock lock; PyRun_SimpleString(cmd.c_str());
};

