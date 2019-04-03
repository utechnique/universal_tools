-- requires module 'options' for build options
require("../source/build/options")
BUILD_ALL = _OPTIONS["build_all"]
BUILD_SAMPLES = BUILD_ALL or _OPTIONS["build_samples"]
BUILD_APP = BUILD_ALL or _OPTIONS["build_app"]

-- android build option
ANDROID_SDK = 9
ANDROID_TARGET = 19

-- target platform
WINDOWS = _ACTION == "vs2015" or _ACTION == "vs2013" or _ACTION == "vs2010" or _ACTION == "vs2008"
LINUX = _ACTION == "gmake"
ANDROID = _OPTIONS["android_ndk"] and true or false
if ANDROID then
    WINDOWS = false
    LINUX = true
end

-- buildname
BUILD_TARGET = _ACTION
if ANDROID then
    BUILD_TARGET = _ACTION .. "_android";
end

-- target types
CONSOLE_APP_KIND = ANDROID and "SharedLib" or "ConsoleApp"
WINDOWED_APP_KIND = ANDROID and "SharedLib" or "WindowedApp"

-- cpp dialect
CPP_STANDARD = _OPTIONS["c03"] and 2003 or 2014
if _ACTION == "vs2013" then
    CPP_STANDARD = 2011
end
if _ACTION == "vs2008" or _ACTION == "vs2010" then
    CPP_STANDARD = 2003
end

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