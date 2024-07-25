#pragma once
#include "atrpch.h"

#include "Loader/Config/Config.h"

#include "VkInfos/VkInfos.h"

namespace ATR
{
    class VkResourceManager
    {
    public:
        VkResourceManager() = default;

        // Configs
        void AbsorbConfigs(const Config& config);

        // Major Components
        void Init();
        void Update();
        void CleanUp();

        // Initializing Vulkan
        void CreateWindow();
        void CreateInstance();
        void SetupDebugMessenger();
        void CreateSurface();
        void SelectPhysicalDevice();
        void CreateLogicalDevice();
        void CreateSwapchain();
        void CreateImageViews();
        void CreateRenderPass();
        void CreateGraphicsPipeline();
        void CreateFrameBuffers();

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

        void GetRequiredExtensions();
        void FindValidationLayers();                            // Validation Layers are specified by the user, not infrastructure

        Bool DeviceSuitable(VkPhysicalDevice device);
        void FindQueueFamilies(VkPhysicalDevice device);
        Bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
        void QuerySwapChainSupport(VkPhysicalDevice device);
        void ConfigureSwapChain(SwapChainSupportDetails support);
        void RetrieveSwapChainImages();

        std::vector<char> ReadShaderCode(const char* path);
        VkShaderModule CreateShaderModule(const std::vector<char>& code);

    private:
        // Configs
        inline static Bool verbose = false;
        UInt width, height;
        Bool enabledValidation;
        std::vector<const char*> instanceExtensions;
        std::vector<const char*> validationLayers;

        const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        /// Vulkan Resources
        // Top-level Vulkan Resources
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;

        // Vulkan Components
        VkPhysicalDevice physicalDevice;
        VkDevice device;
        VkSurfaceKHR surface;
        VkSwapchainKHR swapchain;
        std::array<VkQueue, QueueFamilyIndices::COUNT> queues;
        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainImageViews;
        std::vector<VkFramebuffer> swapchainFrameBuffers;
        VkRenderPass renderPass;
        VkPipelineLayout pipelineLayout;                                // Specify Uniforms
        VkPipeline graphicsPipeline;

        // Customized Infos
        QueueFamilyIndices queueIndices;
        SwapChainSupportDetails swapChainSupport;
        SwapChainConfig swapChainConfig;
        
        /// -----------------

        // GLFW Handle
        GLFWwindow* window;

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
        static inline Float defaultQueuePriority = 0.2f;
    };

}