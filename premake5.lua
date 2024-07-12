workspace "Altrar"
    architecture "x64"
    configurations
    {
        "Debug",   -- dev mode
        "Release"  -- release mode
    }
    startproject "Altrar"
    
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Altrar"    
    location "Altrar"
    kind "ConsoleApp"
    language "C++"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "atrpch.h"
    pchsource "%{prj.name}/src/atrpch.cpp"

    files
    {
        "%{prj.name}/**.h",
        "%{prj.name}/**.cpp"
    }

    includedirs
    {
        "%{prj.name}/src",
        "%{prj.name}/src/Core",
        "%{prj.name}/ext"
    }

    filter "system:Windows"
        cppdialect "C++20"
        staticruntime "On"
        systemversion "latest"

    filter "configurations:Debug"
        defines "ATR_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "ATR_RELEASE"
        optimize "On"