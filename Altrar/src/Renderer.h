#pragma once
#include "atrfwd.h"

#include "Loader/Config/Config.h"
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
        void InitVulkan();				// Vulkan helpers are all in class `VkResources`
        void Update();
        void Cleanup();

    private:
        Config config;
        VkResourceManager vkResources;
    };
}
