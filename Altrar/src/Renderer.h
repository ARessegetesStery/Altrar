#pragma once
#include "atrfwd.h"

#include "Config.h"
#include "VkResources.h"

namespace ATR
{
    class Renderer
    {
    public:
        Renderer(Config&&);

        void Run();

        // Major Components
        void InitRenderer();
        void InitWindow();
        void InitVulkan();				// Vulkan helpers are all in class `VkResources`
        void Update();
        void Cleanup();

    private:
        GLFWwindow* window;

        Config config;
        VkResourceManager vkResources;

    };
}
