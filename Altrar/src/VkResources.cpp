#include "atrpch.h"

#include "VkResources.h"

namespace ATR
{
    void VkResourceManager::AbsorbConfigs(const Config& config)
    {
        VkResourceManager::verbose = config.verbose;
        for (const String& layerName : config.validationLayers)
            this->validationLayers.push_back(layerName.c_str());

        this->width = config.width;
        this->height = config.height;
    }

    void VkResourceManager::Init()
    {
        this->CreateWindow();
        this->CreateInstance();
        this->SetupDebugMessenger();
        this->CreateSurface();
        this->SelectPhysicalDevice();
        this->CreateLogicalDevice();
        this->CreateSwapchain();
        this->CreateImageViews();
        this->CreateRenderPass();
        this->CreateGraphicsPipeline();
    }

    void VkResourceManager::Update()
    {
        while (!glfwWindowShouldClose(this->window))
            glfwPollEvents();
    }

    void VkResourceManager::CleanUp()
    {
        if (enabledValidation)
            this->DestroyDebugUtilsMessengerEXT(this->instance, this->debugMessenger, nullptr);

        vkDestroyRenderPass(this->device, this->renderPass, nullptr);
        vkDestroyPipelineLayout(this->device, this->pipelineLayout, nullptr);
        vkDestroySwapchainKHR(this->device, this->swapchain, nullptr);
        for (const auto& view : swapchainImageViews)
            vkDestroyImageView(this->device, view, nullptr);
        vkDestroyDevice(this->device, nullptr);

        vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
        vkDestroyInstance(this->instance, nullptr);

        glfwDestroyWindow(this->window);
        glfwTerminate();
    }

    void VkResourceManager::CreateWindow()
    {
        ATR_LOG_PART("Initializing Window");
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        this->window = glfwCreateWindow(this->width, this->height, "Altrar", nullptr, nullptr);
        if (!window)
            throw Exception("Failed to create window", ExceptionType::INIT_GLFW);
    }

    void VkResourceManager::CreateInstance()
    {
        ATR_LOG("Creating Vulkan Instance...")
        VkResourceManager::GetRequiredExtensions();
        VkResourceManager::FindValidationLayers();
        
        // Vulkan Instance Creation Info
        // NOTE this cannot be extracted to a function as `appInfo` needs to be available when `VkCreateInstance` is called
        VkApplicationInfo appInfo =
        {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Altrar",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "No Engine",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_0
        };

        VkInstanceCreateInfo createInfo =
        {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
            .enabledExtensionCount = static_cast<UInt>(this->instanceExtensions.size()),
            .ppEnabledExtensionNames = this->instanceExtensions.data()
        };

        if (enabledValidation)
        {
            createInfo.enabledLayerCount = static_cast<UInt>(this->validationLayers.size());
            createInfo.ppEnabledLayerNames = this->validationLayers.data();

            createInfo.pNext = &VkResourceManager::defaultDebugMessengerCreateInfo;
        }
        else
            createInfo.enabledLayerCount = 0;

        if (vkCreateInstance(&createInfo, nullptr, &this->instance) != VK_SUCCESS)
            throw Exception("Failed to create instance", ExceptionType::INIT_VULKAN);
    }

    void VkResourceManager::SetupDebugMessenger()
    {
        ATR_LOG("Setting Up Debug Structures...")
        if (!this->enabledValidation)
            return;

        if (CreateDebugUtilsMessengerEXT(instance, &VkResourceManager::defaultDebugMessengerCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS)
            throw Exception("Failed to set up debug messenger", ExceptionType::INIT_VULKAN);
    }

    void VkResourceManager::CreateSurface()
    {
        ATR_LOG("Creating Window Surface...")
        if (glfwCreateWindowSurface(this->instance, this->window, nullptr, &this->surface) != VK_SUCCESS)
            throw Exception("Failed to create window surface", ExceptionType::INIT_VULKAN);
    }

    void VkResourceManager::SelectPhysicalDevice()
    {
        ATR_LOG("Looking for GPUs...")
        this->physicalDevice = VK_NULL_HANDLE;
        UInt physicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(this->instance, &physicalDeviceCount, nullptr);

        std::vector<VkPhysicalDevice> availablePhysicalDevices(physicalDeviceCount);
        vkEnumeratePhysicalDevices(this->instance, &physicalDeviceCount, availablePhysicalDevices.data());

        if (availablePhysicalDevices.size() == 0)
            throw Exception("No available GPU Found.", ExceptionType::INIT_VULKAN);

        for (auto& device : availablePhysicalDevices)
        {
            if (DeviceSuitable(device))
            {
                this->physicalDevice = device;
                break;
            }
            throw Exception("No Suitable GPU Found.", ExceptionType::INIT_VULKAN);
        }

        if (VkResourceManager::verbose)
        {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(this->physicalDevice, &deviceProperties);
            ATR_PRINT("Using Physical Device: " + String(deviceProperties.deviceName))
            ATR_PRINT("Queue Family Indices: \n" << this->queueIndices)
        }
    }

    void VkResourceManager::CreateLogicalDevice()
    {
        ATR_LOG("Setting Up Device...")
        VkPhysicalDeviceFeatures deviceFeatures = {};

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<UInt> uniqueQueueFamilies;
        for (size_t index = 0; index != QueueFamilyIndices::COUNT; ++index)
            uniqueQueueFamilies.insert(this->queueIndices.indices[index].value());

        for (UInt uniqueQueueIndex : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                //.queueFamilyIndex = this->queueIndices.indices[index].value(),
                .queueFamilyIndex = uniqueQueueIndex,
                .queueCount = 1,
                .pQueuePriorities = &VkResourceManager::defaultQueuePriority
            };
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkDeviceCreateInfo deviceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = static_cast<UInt>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledExtensionCount = (UInt)(this->deviceExtensions.size()),
            .ppEnabledExtensionNames = this->deviceExtensions.data(),
            .pEnabledFeatures = &deviceFeatures
        };

        // NOTE from Vulkan 1.3.290 this is not necessary as device will automatically have the same validation layers as the instance
        if (this->enabledValidation)
        {
            deviceCreateInfo.enabledLayerCount = static_cast<UInt>(this->validationLayers.size());
            deviceCreateInfo.ppEnabledLayerNames = this->validationLayers.data();
        }
        else
            deviceCreateInfo.enabledLayerCount = 0;

        VkResult result = vkCreateDevice(this->physicalDevice, &deviceCreateInfo, nullptr, &this->device);
        if (result != VK_SUCCESS)
            throw Exception("Failed to create logical device", ExceptionType::INIT_VULKAN);

        for (size_t index = 0; index != QueueFamilyIndices::COUNT; ++index)
            vkGetDeviceQueue(this->device, this->queueIndices.indices[index].value(), 0, &this->queues[index]);
    }

    void VkResourceManager::CreateSwapchain()
    {
        ATR_LOG("Creating Swapchain...")
        this->QuerySwapChainSupport(this->physicalDevice);
        this->ConfigureSwapChain(this->swapChainSupport);

        UInt imageCount = this->swapChainSupport.capabilities.minImageCount + 1;
        UInt maxSuportedImages = this->swapChainSupport.capabilities.maxImageCount;
        if (maxSuportedImages != 0 && imageCount > maxSuportedImages)
            imageCount = maxSuportedImages;

        VkSwapchainCreateInfoKHR createInfo = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = this->surface,
            .minImageCount = imageCount,
            .imageFormat = this->swapChainConfig.format.format,
            .imageColorSpace = this->swapChainConfig.format.colorSpace,
            .imageExtent = this->swapChainConfig.extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = this->swapChainSupport.capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = this->swapChainConfig.presentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE
        };

        if (this->queueIndices.indices[QueueFamilyIndices::GRAPHICS] != this->queueIndices.indices[QueueFamilyIndices::PRESENT])
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = this->queueIndices.AllIndices().data();
        }
        else
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateSwapchainKHR(this->device, &createInfo, nullptr, &this->swapchain) != VK_SUCCESS)
            throw Exception("Failed to create swapchain", ExceptionType::INIT_VULKAN);

        this->RetrieveSwapChainImages();
    }

    void VkResourceManager::CreateImageViews()
    {
        ATR_LOG("Retrieving image views from swapchain...")
        swapchainImageViews.resize(swapchainImages.size());
        for (size_t i = 0; i != swapchainImages.size(); ++i)
        {
            VkImageViewCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = swapchainImages[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = swapChainConfig.format.format,

                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY
                },

                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };

            if (vkCreateImageView(this->device, &createInfo, nullptr, &this->swapchainImageViews[i]) != VK_SUCCESS)
                throw Exception("Failed to create swapchain image views", ExceptionType::INIT_VULKAN);
        }
    }

    void VkResourceManager::CreateRenderPass()
    {
        ATR_LOG("Creating Render Pass...")

        VkAttachmentDescription colorAttachment = {
            .format = this->swapChainConfig.format.format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };

        VkAttachmentReference colorAttachmentRef = {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        VkSubpassDescription subpass = {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef
        };

        VkRenderPassCreateInfo renderPass = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &colorAttachment,
            .subpassCount = 1,
            .pSubpasses = &subpass
        };

        if (vkCreateRenderPass(this->device, &renderPass, nullptr, &this->renderPass) != VK_SUCCESS)
            throw Exception("Failed to create render pass", ExceptionType::INIT_PIPELINE);
    }

    void VkResourceManager::CreateGraphicsPipeline()
    {
        ATR_LOG("Creating Graphics Pipeline...")

        // Compile Shaders
        // TODO refactor and define paths in ATRConst.h
        ATR_LOG_SUB("Compiling Shaders...")
        ATR::OS::Execute("scripts/platform/windows/compile-shader.bat");

        std::vector<char> vertShaderCode = ReadShaderCode("bin/shaders/vert.spv");
        std::vector<char> fragShaderCode = ReadShaderCode("bin/shaders/frag.spv");

        // Shader modules can be destroyed after shader stage creation, and therefore is not a member variable of VkResourceManager
        VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

        // Shader Stage Creation
        ATR_LOG_SUB("Creating Shader Stages...")
        VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertShaderModule,
            .pName = Const::DefaultShaderEntryPoint
        };
        VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragShaderModule,
            .pName = Const::DefaultShaderEntryPoint
        };
        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        // Configurable states
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<UInt>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data()
        };

        // Vertex Layout: TODO move the hardcoded values in the shader to the configurable ones here
        ATR_LOG_SUB("Configuring Input Layouts...")
        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 0,
            .vertexAttributeDescriptionCount = 0
        };

        // Input Assembly
        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE
        };

        // Viewport and Scissor (Viewport States)
        ATR_LOG_SUB("Configuring Viewport...")
        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = (Float)this->swapChainConfig.extent.width,
            .height = (Float)this->swapChainConfig.extent.height,
            .minDepth = 0.0f,           // Notice Vulkan uses depth from 0 to 1
            .maxDepth = 1.0f
        };

        VkRect2D scissor = {
            .offset = { 0, 0 },
            .extent = this->swapChainConfig.extent
        };

        // TODO see how this coops with dynamic configuration
        VkPipelineViewportStateCreateInfo viewportState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor
        };

        // Rasterizer
        ATR_LOG_SUB("Configuring Rasterizer...")
        VkPipelineRasterizationStateCreateInfo rasterizer = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .lineWidth = 1.0f
        };

        // Multisampling
        VkPipelineMultisampleStateCreateInfo multisampling = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
        };

        // Color Blending per buffer
        VkPipelineColorBlendAttachmentState colorBlendAttachment = {
            // Enable Alpha Blending
            .blendEnable = VK_TRUE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };

        // Color Blending per Pipeline
        VkPipelineColorBlendStateCreateInfo colorBlending = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachment,
            .blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f }
        };

        // Pipeline Layout for Uniforms
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 0,
            .pSetLayouts = nullptr,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr
        };

        if (vkCreatePipelineLayout(this->device, &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS)
            throw Exception("Failed to create pipeline layout", ExceptionType::INIT_PIPELINE);

        // Cleanup Loaded ShaderCode
        vkDestroyShaderModule(this->device, vertShaderModule, nullptr);
        vkDestroyShaderModule(this->device, fragShaderModule, nullptr);
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL VkResourceManager::DebugCallback(\
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
        VkDebugUtilsMessageTypeFlagsEXT messageType, 
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
        void* pUserData
    )
    {
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            ATR_ERROR(String("[Validation Layer] ") + pCallbackData->pMessage)
        else if (VkResourceManager::verbose)
            ATR_PRINT(String("[Validation Layer] ") + pCallbackData->pMessage)

        return VK_FALSE;
    }

    VkResult VkResourceManager::CreateDebugUtilsMessengerEXT(VkInstance instance, 
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
        const VkAllocationCallbacks* pAllocator, 
        VkDebugUtilsMessengerEXT* pDebugMessenger
    )
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func == nullptr)
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        else
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }

    VkResult VkResourceManager::DestroyDebugUtilsMessengerEXT(
        VkInstance instance, 
        VkDebugUtilsMessengerEXT debugMessenger, 
        const VkAllocationCallbacks* pAllocator
    )
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
            return VK_SUCCESS;
        }
        else
            return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void VkResourceManager::GetRequiredExtensions()
    {
        this->instanceExtensions.clear();

        UInt extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        UInt requiredExtensionCount = 0;
        const char** requiredExtensionNames = glfwGetRequiredInstanceExtensions(&requiredExtensionCount);

        for (UInt i = 0; i != requiredExtensionCount; ++i)
            this->instanceExtensions.push_back(requiredExtensionNames[i]);

        if (enabledValidation)
            instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        if (VkResourceManager::verbose)
        {
            ATR_LOG_SECTION("Vulkan Available Extensions: (" + std::to_string(extensions.size()) + ")")
            for (const auto& extension : extensions)
                ATR_PRINT(String("- ") + extension.extensionName)

            ATR_LOG_SECTION("Vulkan Required Extensions: (" + std::to_string(instanceExtensions.size()) + ")")
            for (const auto& extension : instanceExtensions)
                ATR_PRINT(String("- ") + extension)
        }

        // Check if all required extensions are available
        for (UInt i = 0; i != requiredExtensionCount; ++i)
            if (std::find_if(extensions.begin(), extensions.end(),
                [&](const VkExtensionProperties& ext) { return strcmp(ext.extensionName, requiredExtensionNames[i]) == 0; }
            ) == extensions.cend())
                throw Exception("Required extension not found: " + String(requiredExtensionNames[i]), ExceptionType::INIT_VULKAN);
    }

    void VkResourceManager::FindValidationLayers()
    {
        /// Validation Layers 
        if (enabledValidation)
        {
            UInt validatedLayerCount = 0;
            vkEnumerateInstanceLayerProperties(&validatedLayerCount, nullptr);

            std::vector<VkLayerProperties> availableValidatedLayers(validatedLayerCount);
            vkEnumerateInstanceLayerProperties(&validatedLayerCount, availableValidatedLayers.data());

            if (VkResourceManager::verbose)
            {
                ATR_LOG_SECTION("Vulkan Available Layers: (" + std::to_string(validatedLayerCount) + ")")
                for (const auto& layer : availableValidatedLayers)
                    ATR_PRINT(String("- ") + layer.layerName)

                ATR_LOG_SECTION("Vulkan Required Layers: (" + std::to_string(this->validationLayers.size()) + ")")
                for (const auto& layer : this->validationLayers)
                    ATR_PRINT(String("- ") + layer)
            }

            // Check if all validation layers are available
            for (auto& layerName : this->validationLayers)
                if (std::find_if(availableValidatedLayers.begin(), availableValidatedLayers.end(),
                    [&](const VkLayerProperties& layer) { return strcmp(layer.layerName, layerName) == 0; }
                ) == availableValidatedLayers.cend())
                    throw Exception("Validation layer " + String(layerName) + " not found", ExceptionType::INIT_VULKAN);
        }
    }

    Bool VkResourceManager::DeviceSuitable(VkPhysicalDevice device)
    {
        FindQueueFamilies(device);
        Bool deviceExtensionSupport = CheckDeviceExtensionSupport(device);
        QuerySwapChainSupport(device);

        Bool suitable =
            this->queueIndices.Complete() &&
            deviceExtensionSupport &&
            this->swapChainSupport.Adequate();

        if (suitable)
            return true;
        return false;
    }

    void VkResourceManager::FindQueueFamilies(VkPhysicalDevice device)
    {
        UInt queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        for (UInt i = 0; i != queueFamilies.size(); ++i)
        {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                this->queueIndices.indices[QueueFamilyIndices::GRAPHICS] = i;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->surface, &presentSupport);
            if (presentSupport)
                this->queueIndices.indices[QueueFamilyIndices::PRESENT] = i;

            if (this->queueIndices.Complete())
                break;
        }
    }

    Bool VkResourceManager::CheckDeviceExtensionSupport(VkPhysicalDevice device)
    {
        UInt deviceExtensionCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionCount, nullptr);

        std::vector<VkExtensionProperties> availableDeviceExtensions(deviceExtensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionCount, availableDeviceExtensions.data());

        for (auto& extensionName : this->deviceExtensions)
        {
            if (std::find_if(availableDeviceExtensions.begin(), availableDeviceExtensions.end(),
                [&](const VkExtensionProperties& ext) { return strcmp(ext.extensionName, extensionName) == 0; }
            ) == availableDeviceExtensions.cend())
                return false;
        }

        return true;
    }

    void VkResourceManager::QuerySwapChainSupport(VkPhysicalDevice device)
    {
        // Capabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->surface, &this->swapChainSupport.capabilities);

        // Formats
        UInt formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->surface, &formatCount, nullptr);

        if (formatCount != 0)
        {
            this->swapChainSupport.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->surface, &formatCount, this->swapChainSupport.formats.data());
        }

        // Present Modes
        UInt presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->surface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            this->swapChainSupport.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->surface, &presentModeCount, this->swapChainSupport.presentModes.data());
        }
    }

    void VkResourceManager::ConfigureSwapChain(SwapChainSupportDetails support)
    {
        // Choose Swap Surface Format
        Bool found = false;
        for (const auto& format : support.formats)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                this->swapChainConfig.format = format;
                found = true;
                break;
            }
        }

        if (!found)
            this->swapChainConfig.format = support.formats[0];

        // Choose Swapchain Presentation Mode
        found = false;
        for (const auto& mode : support.presentModes)
        {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                this->swapChainConfig.presentMode = mode;
                found = true;
                break;
            }
        }

        if (!found)
            this->swapChainConfig.presentMode = VK_PRESENT_MODE_FIFO_KHR;

        // Choose Swap Extent
        if (support.capabilities.currentExtent.width != std::numeric_limits<UInt>::max())
            this->swapChainConfig.extent = support.capabilities.currentExtent;
        else
        {
            Int width, height;
            glfwGetFramebufferSize(this->window, &width, &height);

            VkExtent2D actualExtent = {
                .width = static_cast<UInt>(width),
                .height = static_cast<UInt>(height)
            };

            actualExtent.width = std::max(support.capabilities.minImageExtent.width,
                std::min(support.capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(support.capabilities.minImageExtent.height,
                std::min(support.capabilities.maxImageExtent.height, actualExtent.height));

            this->swapChainConfig.extent = actualExtent;
        }
    }

    void VkResourceManager::RetrieveSwapChainImages()
    {
        UInt imageCount = 0;
        vkGetSwapchainImagesKHR(this->device, this->swapchain, &imageCount, nullptr);

        this->swapchainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(this->device, this->swapchain, &imageCount, this->swapchainImages.data());

        if (this->verbose)
            ATR_PRINT("Retrieved " + std::to_string(imageCount) + " images in total in the swapchain.")
    }

    std::vector<char> VkResourceManager::ReadShaderCode(const char* path)
    {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open())
            throw Exception("Failed to open file: " + String(path), ExceptionType::INIT_SHADER);

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    VkShaderModule VkResourceManager::CreateShaderModule(const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = code.size(),
            .pCode = reinterpret_cast<const UInt*>(code.data())
        };

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(this->device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
            throw Exception("Failed to create shader module", ExceptionType::INIT_SHADER);

        return shaderModule;
    }
}