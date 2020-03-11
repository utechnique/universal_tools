-- Generates a project for console application.
-- @options members:
--    projname     - name of the project
--    targname     - target name
--    projdir      - location (folder) of the project
--    incdir       - include directories table
--    consoleapp   - set 'true' if application runs in console mode
--    dependencies - dependencies from other projects
--    def          - preprocessor definitions table
--    charset      - character set, use it only for windows
--    bindfltk     - set 'true' to bind fltk library
--    libs         - libraries to link
--    srcfiles     - source files table
function utApplication(options)
    -- get project location folder from options
    project_folder = options.projdir or "";
    
    -- figure out if application runs in console mode
    console_mode = options.consoleapp or false;
    
    -- main project (premake calls)
    project(options.projname)
    location(BUILD_TARGET .. "/" .. project_folder)
    kind(console_mode and CONSOLE_APP_KIND or WINDOWED_APP_KIND)
    characterset(options.charset or "Unicode")
    targetname(options.targname or options.projname)
    includedirs { options.incdir or {} }
    dependson { options.dependencies or {} }
    defines { options.def or {} }
    links { options.libs or {} }
    files { options.srcfiles or {} }
    
    -- bind fltk
	if options.bindfltk or false then
		USES_FLTK = true
		links "fltk"
		dependson "fltk"
		includedirs { FLTK_INCLUDE_DIR }
		links { FLTK_LIBS }
		defines { FLTK_DEF }
	end
end

