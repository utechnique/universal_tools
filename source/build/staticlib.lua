-- Generates a project for static library.
-- @options members:
--    projname     - name of the project
--    targname     - target name
--    projdir      - location (folder) of the project
--    incdir       - include directories table
--    dependencies - dependencies from other projects
--    def          - preprocessor definitions table
--    charset      - character set, use it only for windows
--    srcfiles     - source files table
--    visualizers  - visualizers for MS Visual Studio
function utStaticLibProj(options)
    project(options.projname)
    location(BUILD_TARGET .. "/" .. (options.projdir or ""))
    kind("StaticLib")
    language("C++")
    targetname(options.targname or options.projname)
    targetdir(BIN_DIR)
    objdir(INTERMEDIATE)
    includedirs { options.incdir or {} }
    dependson { options.dependencies or {} }
    defines { options.def or {} }
    characterset(options.charset or "Default")
    files { options.srcfiles or {} }
	
	 -- bind fltk
	if options.bindfltk or false then
		USES_FLTK = true
		dependson "fltk"
		includedirs { FLTK_INCLUDE_DIR }
		defines { FLTK_DEF }
	end
	
    if SUPPORTS_NATVIS then
        files { options.visualizers or {} }
    end
end