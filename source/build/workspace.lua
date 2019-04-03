function utGenerateWorkspace()
    workspace "UniversalTools"
    -- language is C++ for all projects
    language "C++"
    -- system
    if ANDROID then
        system("android");
    end
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
    -- platfrom
    if os.istarget("Linux") then
        if os.is64bit() then
            platforms { "x64" }
        else
            platforms { "x32" }
        end
    else
        if ANDROID then
            platforms { "ARM" }
        end
        platforms { "x32", "x64" }
    end
    
    -- multithreaded compiling
    flags { "MultiProcessorCompile" }
    
    -- cpp standard
    if LINUX then
        if CPP_STANDARD >= 2014 then
            cppdialect "C++14"
        elseif CPP_STANDARD >= 2011 then
            cppdialect "C++11"
        end
    end
    
    -- standard libraries
    if WINDOWS then
        links { "ws2_32.lib" }
    elseif LINUX then
        -- Librt and Libpthread are included in Bionic lib,
        -- which substitutes GLibC in Android
        if (ANDROID == false) then
            links { "pthread" }
        end
    end
    
    -- platform-specific defines
    if WINDOWS then
        defines { "UT_WINDOWS" }
    elseif LINUX then
        defines { "UT_UNIX" }
		defines { "UT_LINUX" }
        if ANDROID then
            defines { "UT_ANDROID" }
        end
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
        
    filter "platforms:ARM"
        defines { "UT_ARM" }
end