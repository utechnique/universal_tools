-- androidgen module is required for functions:
--    1. utGenerateAndroidManifest
--    2. utGenerateAndroidAntScript
--    3. utGenerateAndroidProperties
--    4. utGenerateAndroidStringRc
--    5. utGenerateAndroidJavaGlue
--    6. utGenerateAndroidCppGlue
require("../source/build/androidgen")


-- Generates a project for console application.
-- @options members:
--    projname     - name of the main cpp project (not Android!)
--    targname     - target name
--    projdir      - location (folder) of the project
--    consoleapp   - set 'true' if application runs in console mode
--    androidsrc   - source files table for the android project
function utAndroidProj(options)
    -- get project location folder from options
    project_folder = options.projdir or "";
    
    -- figure out if application runs in console mode
    console_mode = options.consoleapp or false;
    
    -- generate android package name
    android_pkg_name = options.projname .. "_Pkg"
    
    -- generate cpp-side glue code for the main project
    cpp_glue = utGenerateAndroidCppGlue
    {
        projname = options.projname,
        projdir  = project_folder,
        pkgname  = android_pkg_name
    }
    files(cpp_glue)

    -- android project properties
    project(android_pkg_name)
    location(BUILD_TARGET .. "/" .. project_folder)
    kind("AndroidProj")
    targetname(options.targname or options.projname)
    dependson { options.projname }
    links { options.projname }
    
    -- add source files
    files { options.androidsrc or {} }
    
    -- generate glue for console mode
    if console_mode then
        -- generate manifest file
        android_manifest = utGenerateAndroidManifest
        {
            projname      = options.projname,
            pkgname       = android_pkg_name,
            projdir       = project_folder,
            useinet       = true,
            externstorage = true
        }
        
        -- generate Ant build script
        ant_script = utGenerateAndroidAntScript(project_folder)
        
        -- generate android properties file
        properties_file = utGenerateAndroidProperties(project_folder)
        
        -- generate android resource file for strings.
        string_rc = utGenerateAndroidStringRc(project_folder)
        
        -- generate java-side glue code
        java_glue = utGenerateAndroidJavaGlue
        {
            projname = options.projname,
            projdir  = project_folder,
            pkgname  = android_pkg_name
        }
        
        -- include all generated files
        files { android_manifest, ant_script, properties_file, java_glue, string_rc }
    end
end