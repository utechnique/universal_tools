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

-- check microsoft visual studio version
MSVS_VER = 0
if _ACTION == "vs2022" then
	MSVS_VER = 2022
elseif _ACTION == "vs2019" then
	MSVS_VER = 2019
elseif _ACTION == "vs2017" then
	MSVS_VER = 2017
elseif _ACTION == "vs2015" then
	MSVS_VER = 2015
end

-- compilers
MSVC = _OPTIONS["msvc"] or MSVS_VER ~= 0
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
if _OPTIONS["cpp23"] then
	CPP_STANDARD = 2023
elseif _OPTIONS["cpp20"] then
	CPP_STANDARD = 2020
elseif _OPTIONS["cpp17"] then
	CPP_STANDARD = 2017
elseif _OPTIONS["cpp14"] then
	CPP_STANDARD = 2014
elseif _OPTIONS["cpp11"] then
	CPP_STANDARD = 2011
elseif MSVS_VER ~= 0 then
	if MSVS_VER >= 2022 then
		CPP_STANDARD = 2020
	elseif MSVS_VER >= 2019 then
		CPP_STANDARD = 2017
	elseif MSVS_VER >= 2017 then
		CPP_STANDARD = 2014
	elseif MSVS_VER >= 2015 then
		CPP_STANDARD = 2011
	else
		CPP_STANDARD = 2003
	end
else
    CPP_STANDARD = 2017
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
if GCC then
    table.insert(DBG_LINK_OPTIONS, "-rdynamic")
end

-- FLTK library usage
USES_FLTK = false

-- visual studio .natvis visualizer support
SUPPORTS_NATVIS = false