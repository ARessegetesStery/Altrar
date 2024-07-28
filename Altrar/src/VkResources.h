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
        void UpdateFrame();
        inline Bool ShouldClose() { return glfwWindowShouldClose(this->window); }
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
        void CreateDescriptorSetLayout();
        void CreateGraphicsPipeline();
        void CreateFrameBuffers();
        void CreateCommandPool();
        void CreateDepthBuffer();
        void CreateTextureImage();
        void CreateVertexBuffer();
        void CreateIndexBuffer();
        void CreateUniformBuffer();
        void CreateDescriptorPool();
        void CreateDescriptorSets();
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
        void RetrieveSwapChainImages();

        Bool DeviceSuitable(VkPhysicalDevice device);
        void FindQueueFamilies(VkPhysicalDevice device);
        Bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
        void QuerySwapChainSupport(VkPhysicalDevice device);
        void ConfigureSwapChain(SwapChainSupportDetails support);
        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        void CreateStagingBuffer(VkDeviceSize size, VkBuffer& stagingBuffer, VkDeviceMemory& stagingBufferMemory);
        void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);

        std::vector<char> ReadShaderCode(const char* path);
        VkShaderModule CreateShaderModule(const std::vector<char>& code);

        // Update
        void RecordCommandBuffer(VkCommandBuffer commandBuffer, UInt imageIndex);
        UInt FindMemoryType(UInt typeFilter, VkMemoryPropertyFlags properties);
        void UpdateUniformBuffer(UInt imageIndex);

        // Getter/Setters
        inline String GetUpdateInfo() { return this->updateInfo; }

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
        VkDescriptorSetLayout descriptorSetLayout;
        VkPipelineLayout pipelineLayout;                                // Specify Uniforms
        VkPipeline graphicsPipeline;

        VkCommandPool graphicsCommandPool, transferCommandPool;
        std::vector<VkCommandBuffer> graphicsCommandBuffers;

        VkBuffer vertexBuffer, indexBuffer;                             // Actual buffer on device
        VkDeviceMemory vertexBufferMemory, indexBufferMemory;

        std::vector<VkBuffer> uniformBuffers;
        std::vector<VkDeviceMemory> uniformBuffersMemory;
        std::vector<void*> uniformBufferMappedMemory;

        VkDescriptorPool descriptorPool;
        std::vector<VkDescriptorSet> descriptorSets;                    // Allocated from descriptorPool, similar to commandBuffers from commandPool

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
        Bool frameBufferResized = false;
        String updateInfo = "";

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
            {{-0.5f, -0.5f, 0.f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.f}, {1.0f, 1.0f, 1.0f}},

            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}}
        };

        static inline const std::vector<UInt> indices = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4
        };
    };

}