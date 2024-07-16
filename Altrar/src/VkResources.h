#pragma once
#include "atrpch.h"

#include "Loader/Config/Config.h"

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

        void GetRequiredExtensions();
        void FindValidationLayers();                            // Validation Layers are specified by the user, not infrastructure

        // Cleaning up
        void CleanUp();

        /// Helpers
        // Init
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
        std::vector<const char*> requiredExtensions;
        std::vector<const char*> validationLayers;

        // Vulkan Resources
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;

        // Default Vulkan Configs
        static inline VkDebugUtilsMessengerCreateInfoEXT defaultDebugMessengerCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = VkResourceManager::DebugCallback,
            .pUserData = nullptr
        };
    };

}