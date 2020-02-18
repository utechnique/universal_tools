-- premake5.lua
require("../source/build/overrides")
require("../source/build/buildops")
require("../source/build/workspace")
require("../source/build/staticlib")
require("../source/build/application")
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