-- premake5.lua
require("../source/build/overrides")
require("../source/build/buildops")
require("../source/build/workspace")
require("../source/build/staticlib")
require("../source/build/application")
require("../source/build/input")
require("../source/build/renderapi")
require("../source/build/fltk")

-- override functions
utOverridePremakeFunctions()

-- workspace
utGenerateWorkspace()

-- ut static lib
utStaticLibProj
{
    projname = "ut", -- project name
    targname = "ut", -- target name
    projdir  = "ut", -- project folder
    srcfiles =       -- sources
    {
        "../source/ut/include/**.h",
        "../source/ut/src/**.cpp",
    },
    visualizers = "../source/ut/visualizers/visual_studio/*.natvis"
}

------------------------------------------------------------
--          V I R T U A L   E N V I R O N M E N T         --
------------------------------------------------------------

-- options that are specific for the ve projects
newoption
{
    trigger = "no_ui",
    description = "Builds virtual environment with no user interface"	
}
NO_UI = _OPTIONS["no_ui"]

-- helper library to load images
utStaticLibProj
{
	projname = "stb", -- project name
	targname = "stb", -- target name
	projdir  = "stb", -- project folder
	bindfltk = false, -- using fltk library
	srcfiles =        -- sources
	{
		"../contrib/stb/*.h",
		"../contrib/stb/*.cpp",
	},
}

-- include directories for projects using ve
VE_INCLUDE_DIRS =
{
	"../contrib/stb/",
	"../source/ut/include/",
	"../source/ve/include/",
	RENDER_INCLUDE_DIRS
}

-- libraries for projects using ve,
-- note thet correct order is important for gcc linker
VE_LIBS =
{
	 "ve",
	 "stb",
	 "ut",
	 RENDER_LIBS,
	 INPUT_LIBS 
}

-- libraries with ve resources (icons etc.) for final binaries
VERC_LIBS = {}
VERC_LIBDIRS = {}
if WINDOWS then
    table.insert(VERC_LIBDIRS, "../build/")
    table.insert(VERC_LIBS, "verc")
end

-- ve static lib
utStaticLibProj
{
	projname = "ve",                    -- project name
	targname = "ve",                    -- target name
	projdir  = "ve",                    -- project folder
	bindfltk = NO_UI and false or true, -- using fltk library
	srcfiles =                          -- sources
	{
		"../source/ve/**.h",
		"../source/ve/**.cpp",
	},
	incdir = VE_INCLUDE_DIRS,
	dependencies =       -- dependencies
	{
		"ut"
	},
	def =                -- macros
	{
		RENDER_DEFS
	}
}

-- ve sandbox
utApplication
{
	projname = "ve_sandbox",            -- project name
	projdir  = "ve_sandbox",            -- project folder
	bindfltk = NO_UI and false or true, -- using fltk library
	srcfiles =                          -- sources
	{
		"../source/ve_sandbox/**.h",
		"../source/ve_sandbox/**.cpp",
	},
	incdir =                            --include directories
	{
		VE_INCLUDE_DIRS,
		"../source/ve_sandbox/include/",
	},
	libdir = { RENDER_LIB_DIRS, VERC_LIBDIRS }, --library directories
	libdir_32 = { RENDER_LIB_DIRS_32 }, --library directories (32 bits only)
	libdir_64 = { RENDER_LIB_DIRS_64 }, --library directories (64 bits only)
	libs = { VE_LIBS, VERC_LIBS }, -- libraries (same names for 32 and 64 bit versions)
	libs_release = { RENDER_LIBS_RELEASE }, -- libraries ('Release' only)
	libs_dbg = { RENDER_LIBS_DBG }, -- libraries ('Debug' only)
	dependencies = { "ve", "stb", "ut" }, -- dependencies
	def = { RENDER_DEFS } -- preprocessor
}

------------------------------------------------------------
--                     S A M P L E S                      --
------------------------------------------------------------

if BUILD_SAMPLES then
    utApplication
    {
        projname   = "compatibility_test", -- project name
        projdir    = "samples",            -- project folder
        consoleapp = true,                 -- console application
        srcfiles   =                       -- sources
        {
            "../source/samples/compatibility_test/*.h",
            "../source/samples/compatibility_test/*.cpp"
        },
        libs =                             -- libraries
        {
            "ut"
        },
        dependencies =                     -- dependencies
        {
            "ut"
        }
    }
end

------------------------------------------------------------
--                        F L T K                         --
------------------------------------------------------------

if USES_FLTK then
    utFLTK()
end