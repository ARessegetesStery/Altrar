#pragma once
#include "atrpch.h"

#include "Config.h"

namespace ATR
{
    class VkResourceManager
    {
    public:
        VkResourceManager() = default;

        // Configs
        void AbsorbConfigs(const Config& config);

        // Initializing Vulkan
        void CreateInstance();

        // Cleaning up
        void CleanUp();

    private:
        // Configs
        Bool verbose;
        Bool enableValidation;
        std::vector<const char*> validationLayers;

        // Vulkan Resources
        VkInstance instance;
    };

}