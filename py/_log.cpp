/*************************************************************************
*  2006-2008 © Václav Šmilauer                                           *
*  2019      © Janek Kozicki, rewritten using boost::log the version     *
*              which was removed in 2008 (git revision 014b11496)        *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

// hint: follow changes in d067b0696a8 to add new modules.

#include<core/Omega.hpp>
#include<lib/pyutil/doc_opts.hpp>
#include<lib/base/Logging.hpp>
#include<string>

CREATE_CPP_LOCAL_LOGGER("_log.cpp");

namespace py = boost::python;

void printNoBoostLogWarning() {
	std::cerr << "\nWarning: yade was compiled with cmake option -DENABLE_LOGGER=OFF, any attempts to manipulate log filter levels will not have effect.\n\n";
}

int getDefaultLogLevel() {
#ifdef YADE_BOOST_LOG
	return Logging::instance().getDefaultLogLevel();
#else
	return std::min(MAX_LOG_LEVEL,MAX_HARDCODED_LOG_LEVEL);
#endif
}

void setDefaultLogLevel(int level) {
#ifdef YADE_BOOST_LOG
	Logging::instance().setNamedLogLevel("Default",level);
#else
	printNoBoostLogWarning();
#endif
}

void testAllLevels() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
// ignore unused variable warning because when log level is low, they are not used (not printed).
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
	int testInt     = 0;
	std::string testStr = "test string";
	Real testReal(11);
	Vector3r testVec(1,2,3);
	Matrix3r testMat = (Matrix3r() << 1, 2, 3, 4, 5, 6, 7, 8, 9).finished();
	std::complex<Real> testComplex(-1,1);

	LOG_0_NOFILTER("Test log level: LOG_0_NOFILTER, test int: " << testInt++);
	LOG_1_FATAL   ("Test log level: LOG_1_FATAL, test int: " << testInt++);
	LOG_2_ERROR   ("Test log level: LOG_2_ERROR, test int: " << testInt++);
	LOG_3_WARN    ("Test log level: LOG_3_WARN, test int: " << testInt++);
	LOG_4_INFO    ("Test log level: LOG_4_INFO, test int: " << testInt++);
	LOG_5_DEBUG   ("Test log level: LOG_5_DEBUG, test int: " << testInt++);
	LOG_6_TRACE   ("Test log level: LOG_6_TRACE, test int: " << testInt++);

	LOG_0_NOFILTER("Below 6 variables are printed at filter level TRACE, then macro TRACE; is used");
	TRVAR1(testInt);
	TRVAR2(testInt,testStr);
	TRVAR3(testInt,testStr,testReal);
	TRVAR4(testInt,testStr,testReal,testVec);
	TRVAR5(testInt,testStr,testReal,testVec,testMat);
	TRVAR6(testInt,testStr,testReal,testVec,testMat,testComplex);

	TRACE;
#pragma GCC diagnostic pop
}

// accepted streams: "clog", "cerr", "cout", "filename"
// It is possible to set different levels per log file, see notes about that in Logging::setOutputStream(…)
void setOutputStream(std::string streamName, bool reset /*, int level */ ) {
#ifdef YADE_BOOST_LOG
	Logging::instance().setOutputStream( streamName , reset );
	if(reset) {
		LOG_INFO("Log output stream has been set to "<< streamName <<". Other output streams were removed.");
	} else {
		LOG_INFO("Additional output stream has been set to "<< streamName <<".");
	}
#else
	printNoBoostLogWarning();
#endif
}

void resetOutputStream() {
#ifdef YADE_BOOST_LOG
	Logging::instance().setOutputStream("clog" , true);
	LOG_INFO("Log output stream has been reset to std::clog. File sinks are not removed.");
#else
	printNoBoostLogWarning();
#endif
}

void setLevel(std::string className, int level) {
#ifdef YADE_BOOST_LOG
	Logging::instance().setNamedLogLevel(className , level);
	LOG_INFO("filter log level for " << className << " has been set to " << Logging::instance().getNamedLogLevel(className));
#else
	printNoBoostLogWarning();
#endif
}

void unsetLevel(std::string className) {
#ifdef YADE_BOOST_LOG
	Logging::instance().unsetNamedLogLevel(className);
	LOG_INFO("filter log level for " << className << " has been unset to " << Logging::instance().getNamedLogLevel(className));
#else
	printNoBoostLogWarning();
#endif
}

py::dict getAllLevels() {
	py::dict ret{};
#ifdef YADE_BOOST_LOG
	for(const auto& a : Logging::instance().getClassLogLevels()) {
		ret[a.first]=a.second;
	}
#else
	printNoBoostLogWarning();
#endif
	return ret;
}

py::dict getUsedLevels() {
	py::dict ret{};
#ifdef YADE_BOOST_LOG
	for(const auto& a : Logging::instance().getClassLogLevels()) {
		if(a.second != -1) {
			ret[a.first]=a.second;
		}
	}
#else
	printNoBoostLogWarning();
#endif
	return ret;
}

void setUseColors(bool use) {
#ifdef YADE_BOOST_LOG
	Logging::instance().setUseColors(use);
#else
	printNoBoostLogWarning();
#endif
}

void readConfigFile(std::string fname){
#ifdef YADE_BOOST_LOG
	Logging::instance().readConfigFile(fname);
#else
	printNoBoostLogWarning();
#endif
}

void saveConfigFile(std::string fname) {
#ifdef YADE_BOOST_LOG
	Logging::instance().saveConfigFile(fname);
#else
	printNoBoostLogWarning();
#endif
}

std::string defaultConfigFileName() {
#ifdef YADE_BOOST_LOG
	return Logging::instance().defaultConfigFileName();
#else
	printNoBoostLogWarning();
	return "";
#endif
}

BOOST_PYTHON_MODULE(_log){
	YADE_SET_DOCSTRING_OPTS;
// We can use C++ string literal just like """ """ in python to write docstrings (see. https://en.cppreference.com/w/cpp/language/string_literal )
// The """ is a custom delimeter, we could use    R"RAW( instead, or any other delimeter. This decides what will be the termination delimeter.
// The docstrings can use syntax :param ……: ……… :return: ……. For details see https://thomas-cokelaer.info/tutorials/sphinx/docstring_python.html
	py::def("testAllLevels", testAllLevels, R"""(
This function prints test messages on all log levels. Can be used to see how filtering works and to what streams the logs are written.
	)""");
	py::def("getDefaultLogLevel", getDefaultLogLevel, R"""(
:return: The current ``Default`` filter log level.
	)""");
	py::def("setDefaultLogLevel", setDefaultLogLevel, R"""(
:param int level: Sets the ``Default`` filter log level, same as calling ``log.setLevel("Default",level)``.
	)""");
	py::def("setOutputStream", setOutputStream, R"""(
:param str streamName: sets the output stream, special names ``cout``, ``cerr``, and default ``clog`` use the ``std::cout``, ``std::cerr``, ``std::clog`` counterpart. Every other name means that log will be written to a filename.
:param bool reset: dictates whether all previously set output streams are to be removed. When set to false: the new output stream is set additionally to the current one.
	)""");
	py::def("resetOutputStream", resetOutputStream, R"""(
Resets log output stream to default state: all logs are printed on ``std::clog`` channel, which usually redirects to ``std::cerr``.
	)""");
	py::def("setLevel", setLevel , R"""(
Set filter level (constants ``TRACE`` (6), ``DEBUG`` (5), ``INFO`` (4), ``WARN`` (3), ``ERROR`` (2), ``FATAL`` (1), ``NOFILTER`` (0)) for given logger.

:param str className: The logger name for which the filter level is to be set. Use name ``Default`` to change the default filter level.
:param int level: The filter level to be set.
.. warning:: setting ``Default`` log level higher than ``MAX_LOG_LEVEL`` provided during compilation will have no effect. Logs will not be printed because they are removed during compilation.
	)""");
	py::def("unsetLevel", unsetLevel , R"""(
:param str className: The logger name for which the filter level is to be unset, so that a ``Default`` will be used instead. Unsetting the ``Default`` level will change it to max level and print everything.
	)""");
	py::def("getAllLevels", getAllLevels , R"""(
:return: A python dictionary with all known loggers in yade. Those without a debug level set will have value -1 to indicate that ``Default`` filter log level is to be used for them.
	)""");
	py::def("getUsedLevels", getUsedLevels , R"""(
:return: A python dictionary with all used log levels in yade. Those without a debug level (value -1) are omitted.
	)""");
	py::def("setUseColors", setUseColors, R"""(
Turn on/off colors in log messages. By default is on. When logging to file it is better turned off.
	)""");

// Config file
	py::def("readConfigFile", readConfigFile, R"""(
Loads the given configuration file.

:param str fname: the config file to be loaded.
	)""");
	py::def("saveConfigFile", saveConfigFile, R"""(
Saves log config to specified file.

:param str fname: the config file to be saved.
	)""");
	py::def("defaultConfigFileName", defaultConfigFileName, R"""(
:return: the default log config file, which is loaded at startup.
	)""");

	py::scope().attr("TRACE")=int(6);
	py::scope().attr("DEBUG")=int(5);
	py::scope().attr("INFO") =int(4);
	py::scope().attr("WARN") =int(3);
	py::scope().attr("ERROR")=int(2);
	py::scope().attr("FATAL")=int(1);
	py::scope().attr("NOFILTER")=int(0);
}

/* this was in git revision 014b11496

BOOST_PYTHON_MODULE(log){
	python::def("loadConfig",logLoadConfig,(python::arg("fileName")),"Load configuration from file (log4cxx::PropertyConfigurator::configure)");
}

*/
