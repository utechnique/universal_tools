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
--    libdir       - library directories
--    libdir_32    - library directories specific only for 32 bit platform
--    libdir_64    - library directories specific only for 32 bit platform
--    libs         - libraries to link
--    libs_release - libraries to link (for 'Release' configuration only)
--    libs_dbg     - libraries to link (for 'Debug' configuration only)
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
	libdirs { options.libdir or {} }
    links { options.libs or {} }
    files { options.srcfiles or {} }

	 -- bind fltk
	if options.bindfltk or false then
		USES_FLTK = true
		links { "fltk" }
		dependson "fltk"
		includedirs { FLTK_INCLUDE_DIR }
		links { FLTK_LIBS }
		defines { FLTK_DEF }
	end

	-- filters
	filter "platforms:x32"
        libdirs { options.libdir_32 or {} }
	filter "platforms:x64"
        libdirs { options.libdir_64 or {} }
	filter "configurations:Debug"
        links { options.libs_dbg or {} }
	filter "configurations:Release"
        links { options.libs_release or {} }
end

