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

------------------------------------------------------------
--                V I R T U A L   M I N D                 --
------------------------------------------------------------

-- libraries with vm resources (icons etc.) for final binaries
VMRC_LIBS = {}
VMRC_LIBDIRS = {}
if WINDOWS then
    table.insert(VMRC_LIBDIRS, "../build/")
    table.insert(VMRC_LIBS, "vmrc")
end

-- vm static lib
utStaticLibProj
{
	projname = "vm",                    -- project name
	targname = "vm",                    -- target name
	projdir  = "vm",                    -- project folder
	bindfltk = NO_UI and false or true, -- using fltk library
	srcfiles =                          -- sources
	{
		"../source/vm/**.h",
		"../source/vm/**.cpp",
	},
	incdir =                            -- includes
	{
		"../source/ut/include/",
	},
	dependencies =                      -- dependencies
	{
		"ut"
	}
}

-- vm cradle
utApplication
{
	projname = "cradle",                -- project name
	projdir  = "cradle",                -- project folder
	bindfltk = NO_UI and false or true, -- using fltk library
	srcfiles =                          -- sources
	{
		"../source/cradle/**.h",
		"../source/cradle/**.cpp",
	},
	incdir =                            --include directories
	{
		VE_INCLUDE_DIRS,
		"../source/vm/include/",
		"../source/cradle/include/",
	},
	libdir = { RENDER_LIB_DIRS, VMRC_LIBDIRS }, --library directories
	libdir_32 = { RENDER_LIB_DIRS_32 }, --library directories (32 bits only)
	libdir_64 = { RENDER_LIB_DIRS_64 }, --library directories (64 bits only)
	libs = { VE_LIBS, VMRC_LIBS, "vm" }, -- libraries (same names for 32 and 64 bit versions)
	libs_release = { RENDER_LIBS_RELEASE }, -- libraries ('Release' only)
	libs_dbg = { RENDER_LIBS_DBG }, -- libraries ('Debug' only)
	dependencies = { "ve", "stb", "ut" }, -- dependencies
	def = { RENDER_DEFS } -- preprocessor
}

------------------------------------------------------------
--                        F L T K                         --
------------------------------------------------------------

if USES_FLTK then
    utFLTK()
end