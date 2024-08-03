function utGenerateWorkspace()
    workspace "UniversalTools"
    -- language is C++ for all projects
    language "C++"
    -- directories
    targetdir(BIN_DIR)
    objdir(INTERMEDIATE)
    debugdir(BIN_DIR)
    includedirs { "../source/ut/include" }
    libdirs { BIN_DIR }
    -- configuration
    configurations { "Debug", "Release" }
    -- location
    location(BUILD_TARGET)
    -- ISA platfrom
	if X86 then
		platforms { "x32" }
	end
	if X64 then
		platforms { "x64" }
	end
    
    -- multithreaded compiling
    flags { "MultiProcessorCompile" }
    
    -- cpp standard
	if CPP_STANDARD >= 2023 then
		cppdialect "C++23"
	elseif CPP_STANDARD >= 2020 then
		cppdialect "C++20"
	elseif CPP_STANDARD >= 2017 then
		cppdialect "C++17"
	elseif CPP_STANDARD >= 2014 then
		cppdialect "C++14"
	elseif CPP_STANDARD >= 2011 then
		cppdialect "C++11"
	end
    
    -- standard libraries
    if WINDOWS then
        links { "ws2_32.lib" }
    elseif LINUX then
        -- Librt and Libpthread are included in Bionic lib
		links { "pthread" }
		linkoptions { "-pthread" }
    end
    
    -- platform-specific defines
    if WINDOWS then
        defines { "UT_WINDOWS" }
    elseif LINUX then
        defines { "UT_UNIX" }
		defines { "UT_LINUX" }
    end
    
    -- C++ standard macro-definition
    defines { "CPP_STANDARD="..CPP_STANDARD }
    
    -- debug configuration
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        links { DBG_LIBS }
        linkoptions { DBG_LINK_OPTIONS }
    
    -- release configuration
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        
    -- processor architecture
    filter "platforms:x32"
        defines { "UT_X32" }
    
    filter "platforms:x64"
        defines { "UT_X64" }
end