#pragma once
#include "atrpch.h"

#include "Loader/Config/Config.h"

#include "Geometry/Geometry.h"
#include "VkInfos/VkInfos.h"

namespace ATR
{
    class VkResourceManager
    {
    public:
        VkResourceManager() = default;                  // No explicit constructor needed, init members in declaration

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
        void CreateCommandPool();
        void CreateVertexBuffer();
        void CreateCommandBuffer();
        void CreateSyncGadgets();

        // Update
        void DrawFrame();
        void RecreateSwapchain();

        // Clean Up
        void CleanUpSwapchain();

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
        void RecordCommandBuffer(VkCommandBuffer commandBuffer, UInt imageIndex);
        UInt FindMemoryType(UInt typeFilter, VkMemoryPropertyFlags properties);

        std::vector<char> ReadShaderCode(const char* path);
        VkShaderModule CreateShaderModule(const std::vector<char>& code);

    private:
        // Configs
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
        VkCommandPool graphicsCommandPool;
        std::vector<VkCommandBuffer> graphicsCommandBuffers;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;

        // Synchronization gadgets
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;

        // Customized Infos
        QueueFamilyIndices queueIndices;
        SwapChainSupportDetails swapChainSupport;
        SwapChainConfig swapChainConfig;
        
        /// -----------------

        // GLFW Handle
        GLFWwindow* window;

        // Per-update Invariances
        UInt currentFrameIndex = 0;
        UInt currentFrameNumber = 0;
        Bool frameBufferResized = false;

        // Static (global) functions
        // TODO may want to have a separate class & files to handle callbacks
        static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

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
        static inline constexpr Float defaultQueuePriority = 0.2f;
        static inline constexpr VkClearValue defaultClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
        static inline constexpr UInt maxFramesInFlight = 2;

        // Temporary Global Variables
        static inline const std::vector<Vertex> vertices = {
            {{ 0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
            {{ 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}}
        };
    };

}