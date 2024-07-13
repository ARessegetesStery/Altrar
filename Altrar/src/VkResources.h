#pragma once
#include "atrpch.h"


namespace ATR
{
    class VkResourceManager
    {
    public:
        VkResourceManager() = default;

        // Initializing Vulkan
        void CreateInstance();

        // Cleaning up
        void CleanUp();

    private:
        VkInstance instance;
    };

}