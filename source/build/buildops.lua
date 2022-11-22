-- requires module 'options' for build options
require("../source/build/options")
BUILD_ALL = _OPTIONS["build_all"]
BUILD_SAMPLES = BUILD_ALL or _OPTIONS["build_samples"]
BUILD_APP = BUILD_ALL or _OPTIONS["build_app"]

-- processor instruction set architecture
X64 = _OPTIONS["x64"]
X86 = _OPTIONS["x86"]
if not X64 and not X86 then
	X64 = true
	X86 = true
end

-- compilers
MSVC = _OPTIONS["msvc"] or _ACTION == "vs2019" or _ACTION == "vs2017" or _ACTION == "vs2015" or _ACTION == "vs2013" or _ACTION == "vs2010" or _ACTION == "vs2008"
GCC = _OPTIONS["gcc"] or _ACTION == "gmake"

-- target platform
WINDOWS = _OPTIONS["windows"] or MSVC
LINUX = _OPTIONS["linux"] or GCC

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
if GCC then
    table.insert(DBG_LINK_OPTIONS, "-rdynamic")
end

-- FLTK library usage
USES_FLTK = false

-- visual studio .natvis visualizer support
SUPPORTS_NATVIS = WINDOWS and (_ACTION ~= "vs2008" and _ACTION ~= "vs2010")