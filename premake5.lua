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
    language "c++"
    cppdialect "c++20"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "atrpch.h"
    pchsource "%{prj.name}/src/atrpch.cpp"

    staticruntime "off"             -- set to off to avoid weird compatibility issues

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "%{prj.name}/src",
        "%{prj.name}/src/Core",
        "%{prj.name}/ext"
    }

    libdirs
    {
        "%{prj.name}/ext/glfw/lib",
        "%{prj.name}/ext/vulkan/lib"
    }

    links
    {
        "glfw3",
        "vulkan-1"
    }

    filter "system:Windows"
        staticruntime "off"
        systemversion "latest"

    filter "configurations:Debug"
        defines "ATR_DEBUG"
        symbols "on"

    filter "configurations:Release"
        defines "ATR_RELEASE"
        optimize "on"