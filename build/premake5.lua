-- premake5.lua
require("../source/build/overrides")
require("../source/build/buildops")
require("../source/build/workspace")
require("../source/build/staticlib")
require("../source/build/application")
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
	incdir =
	{
		"../source/ut/include/",
		"../source/ve/include/",
		RENDER_INCLUDE_DIRS
	},
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
		"../source/ut/include/",
		"../source/ve/include/",
		"../source/ve_sandbox/include/",
		RENDER_INCLUDE_DIRS
	},
	libdir = { RENDER_LIB_DIRS }, --library directories
	libdir_32 = { RENDER_LIB_DIRS_32 }, --library directories (32 bits only)
	libdir_64 = { RENDER_LIB_DIRS_64 }, --library directories (64 bits only)
	libs = { "ve", "ut", RENDER_LIBS }, -- libraries (correct order is important for gcc linker)
	libs_release = { RENDER_LIBS_RELEASE }, -- libraries ('Release' only)
	libs_dbg = { RENDER_LIBS_DBG }, -- libraries ('Debug' only)
	dependencies = { "ve", "ut" }, -- dependencies
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