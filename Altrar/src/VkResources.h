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
        void Init();

        void CreateInstance();
        void SetupDebugMessenger();

        // Cleaning up
        void CleanUp();

        // Helpers
        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);
        VkResult CreateDebugUtilsMessengerEXT(
            VkInstance instance,
            const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator,
            VkDebugUtilsMessengerEXT* pDebugMessenger
        );
        VkResult DestroyDebugUtilsMessengerEXT(
            VkInstance instance,
            VkDebugUtilsMessengerEXT debugMessenger,
            const VkAllocationCallbacks* pAllocator
        );

    private:
        // Configs
        Bool verbose;
        Bool enabledValidation;
        std::vector<const char*> validationLayers;

        // Vulkan Resources
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
    };

}