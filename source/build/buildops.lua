-- requires module 'options' for build options
require("../source/build/options")
BUILD_ALL = _OPTIONS["build_all"]
BUILD_SAMPLES = BUILD_ALL or _OPTIONS["build_samples"]
BUILD_APP = BUILD_ALL or _OPTIONS["build_app"]

-- target platform
WINDOWS = _ACTION == "vs2015" or _ACTION == "vs2013" or _ACTION == "vs2010" or _ACTION == "vs2008"
LINUX = _ACTION == "gmake"

-- buildname
BUILD_TARGET = _ACTION

-- target types
CONSOLE_APP_KIND = "ConsoleApp"
WINDOWED_APP_KIND = "WindowedApp"

-- cpp dialect
CPP_STANDARD = 2011

-- intermediate directory
INTERMEDIATE = "../intermediate/%{cfg.buildcfg}/" ..BUILD_TARGET.. "/%{cfg.architecture}"

-- output directory
BIN_DIR = "../bin/%{cfg.buildcfg}/" ..BUILD_TARGET.. "/%{cfg.architecture}"

-- libraries
DBG_LIBS = {}
if WINDOWS then
    table.insert(DBG_LIBS, "Dbghelp")
elseif LINUX then
    table.insert(DBG_LIBS, "dl")
end

-- link options
DBG_LINK_OPTIONS = {}
if LINUX then
    table.insert(DBG_LINK_OPTIONS, "-rdynamic")
end

-- FLTK library usage
USES_FLTK = false

-- visual studio .natvis visualizer support
SUPPORTS_NATVIS = WINDOWS and (_ACTION ~= "vs2008" and _ACTION ~= "vs2010")