#include "atrpch.h"

#include "VkResources.h"

namespace ATR
{
    void VkResourceManager::AbsorbConfigs(const Config& config)
    {
        for (const String& layerName : config.validationLayers)
            this->validationLayers.push_back(layerName.c_str());

        this->width = config.width;
        this->height = config.height;
    }

    void VkResourceManager::Init()
    {
        // Setup GLFW Window
        this->CreateWindow();

        // Setup Vulkan
        this->CreateInstance();
        this->SetupDebugMessenger();
        this->CreateSurface();
        this->SelectPhysicalDevice();
        this->CreateLogicalDevice();

        // Setup Graphics Pipeline
        this->CreateSwapchain();
        this->CreateImageViews();
        this->CreateRenderPass();
        this->CreateGraphicsPipeline();
        this->CreateFrameBuffers();
        this->CreateCommandPool();
        this->CreateVertexBuffer();
        this->CreateCommandBuffer();
        this->CreateSyncGadgets();
    }

    void VkResourceManager::Update()
    {
        // TODO move the main loop to Renderer, and encapsulate the update and closing condition as methods
        while (!glfwWindowShouldClose(this->window))
        {
            ATR_LOG_VERBOSE("Rendering Frame " << this->currentFrameNumber << " [" << this->currentFrameIndex << "]")
                ++this->currentFrameNumber;
            this->DrawFrame();
            glfwPollEvents();
        }
    }

    void VkResourceManager::CleanUp()
    {
        // Ensure that the device is subject to cleaning up
        vkDeviceWaitIdle(this->device);

        // Clean up validation layer
        if (enabledValidation)
            this->DestroyDebugUtilsMessengerEXT(this->instance, this->debugMessenger, nullptr);

        // Clean up synchronization gadgets
        for (auto& semaphore : this->imageAvailableSemaphores)
            vkDestroySemaphore(this->device, semaphore, nullptr);
        for (auto& semaphore : this->renderFinishedSemaphores)
            vkDestroySemaphore(this->device, semaphore, nullptr);
        for (auto& fence : this->inFlightFences)
            vkDestroyFence(this->device, fence, nullptr);

        // Clean up device-dependent resources
        vkDestroyBuffer(this->device, this->vertexBuffer, nullptr);
        vkFreeMemory(this->device, this->vertexBufferMemory, nullptr);                      // This must be done after the buffer itself has been freed
        vkDestroyCommandPool(this->device, this->graphicsCommandPool, nullptr);             // Command buffers are automatically freed when we free the command pool
        vkDestroyPipeline(this->device, this->graphicsPipeline, nullptr);
        vkDestroyRenderPass(this->device, this->renderPass, nullptr);
        vkDestroyPipelineLayout(this->device, this->pipelineLayout, nullptr);
        this->CleanUpSwapchain();
        vkDestroyDevice(this->device, nullptr);
        
        // Clean up instance-dependent resources
        vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
        vkDestroyInstance(this->instance, nullptr);

        // Clean up GLFW resources
        glfwDestroyWindow(this->window);
        glfwTerminate();
    }

    void VkResourceManager::CreateWindow()
    {
        ATR_LOG_PART("Initializing Window");
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        this->window = glfwCreateWindow(this->width, this->height, "Altrar", nullptr, nullptr);
        glfwSetWindowUserPointer(this->window, this);
        glfwSetFramebufferSizeCallback(this->window, VkResourceManager::FramebufferResizeCallback);
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

        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(this->physicalDevice, &deviceProperties);
        ATR_PRINT_VERBOSE("Using Physical Device: " + String(deviceProperties.deviceName))
        ATR_PRINT_VERBOSE("Queue Family Indices: \n" << this->queueIndices)
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

        VkSubpassDependency dependency = {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        };

        VkRenderPassCreateInfo renderPass = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &colorAttachment,
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &dependency
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
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &Vertex::bindingDescription,
            .vertexAttributeDescriptionCount = Vertex::attributeDescriptions.size(),
            .pVertexAttributeDescriptions = Vertex::attributeDescriptions.data(),
        };

        // Input Assembly
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {
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
        VkPipelineViewportStateCreateInfo viewportStateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor
        };

        // Rasterizer
        ATR_LOG_SUB("Configuring Rasterizer...")
        VkPipelineRasterizationStateCreateInfo rasterizerInfo = {
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
        VkPipelineMultisampleStateCreateInfo multisamplingInfo = {
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
        VkPipelineColorBlendStateCreateInfo colorBlendingInfo = {
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

        // Creating the Graphics Pipeline
        VkGraphicsPipelineCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = 2,
            .pStages = shaderStages,
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssemblyInfo,
            .pViewportState = &viewportStateInfo,
            .pRasterizationState = &rasterizerInfo,
            .pMultisampleState = &multisamplingInfo,
            .pColorBlendState = &colorBlendingInfo,
            .pDynamicState = &dynamicStateCreateInfo,
            .layout = this->pipelineLayout,
            .renderPass = this->renderPass, 
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1
        };

        if (vkCreateGraphicsPipelines(this->device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &this->graphicsPipeline) != VK_SUCCESS)
            throw Exception("Failed to create graphics pipeline", ExceptionType::INIT_PIPELINE);

        ATR_LOG("Graphics Pipeline Created Successfully.")

        // Cleanup Loaded ShaderCode
        vkDestroyShaderModule(this->device, vertShaderModule, nullptr);
        vkDestroyShaderModule(this->device, fragShaderModule, nullptr);
    }

    void VkResourceManager::CreateFrameBuffers()
    {
        ATR_LOG("Creating Framebuffers...")

        this->swapchainFrameBuffers.resize(this->swapchainImageViews.size());

        for (size_t i = 0; i != this->swapchainImageViews.size(); ++i)
        {
            VkImageView attachments[] = { this->swapchainImageViews[i] };

            VkFramebufferCreateInfo framebufferInfo = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = this->renderPass,
                .attachmentCount = 1,
                .pAttachments = attachments,
                .width = this->swapChainConfig.extent.width,
                .height = this->swapChainConfig.extent.height,
                .layers = 1
            };

            if (vkCreateFramebuffer(this->device, &framebufferInfo, nullptr, &this->swapchainFrameBuffers[i]) != VK_SUCCESS)
                throw Exception("Failed to create framebuffer", ExceptionType::INIT_PIPELINE);
        }
    }

    void VkResourceManager::CreateCommandPool()
    {
        ATR_LOG("Creating Command Pool...")
        VkCommandPoolCreateInfo commandPoolInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = this->queueIndices.indices[QueueFamilyIndices::GRAPHICS].value()
        };

        if (vkCreateCommandPool(this->device, &commandPoolInfo, nullptr, &this->graphicsCommandPool) != VK_SUCCESS)
            throw Exception("Failed to create command pool", ExceptionType::INIT_PIPELINE);
    }

    void VkResourceManager::CreateVertexBuffer()
    {
        // TODO make this more flexible
        ATR_LOG("Creating Vertex Buffer...")
        
        VkDeviceSize bufferSize = sizeof(VkResourceManager::vertices[0]) * VkResourceManager::vertices.size();
        this->CreateBuffer(
            bufferSize, 
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            // Host coherency implies that changes to the memory on the host will NOTIFY (but not necessarily immediately visible to) the device
            // Device visibility depends on its actual memory model and is ensured by the driver
            this->vertexBuffer, 
            this->vertexBufferMemory
        );

        void* data;
        vkMapMemory(this->device, this->vertexBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, VkResourceManager::vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(this->device, this->vertexBufferMemory);
    }

    void VkResourceManager::CreateCommandBuffer()
    {
        ATR_LOG("Creating Command Buffer...")
        this->graphicsCommandBuffers.resize(VkResourceManager::maxFramesInFlight);
        VkCommandBufferAllocateInfo commandBufferInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = this->graphicsCommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = static_cast<UInt>(this->graphicsCommandBuffers.size())
        };

        if (vkAllocateCommandBuffers(this->device, &commandBufferInfo, graphicsCommandBuffers.data()) != VK_SUCCESS)
            throw Exception("Failed to allocate command buffers", ExceptionType::INIT_PIPELINE);
    }

    void VkResourceManager::CreateSyncGadgets()
    {
        ATR_LOG("Creating Synchronization Gadgets...")

        this->imageAvailableSemaphores.resize(VkResourceManager::maxFramesInFlight);
        this->renderFinishedSemaphores.resize(VkResourceManager::maxFramesInFlight);
        this->inFlightFences.resize(VkResourceManager::maxFramesInFlight);

        VkSemaphoreCreateInfo semaphoreInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };
        VkFenceCreateInfo fenceInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        for (UInt i = 0; i != VkResourceManager::maxFramesInFlight; ++i)
        {
            if (vkCreateSemaphore(this->device, &semaphoreInfo, nullptr, &this->imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(this->device, &semaphoreInfo, nullptr, &this->renderFinishedSemaphores[i]) != VK_SUCCESS)
                throw Exception("Failed to create synchronization gadgets: semaphores", ExceptionType::INIT_PIPELINE);

            if (vkCreateFence(this->device, &fenceInfo, nullptr, &this->inFlightFences[i]) != VK_SUCCESS)
                throw Exception("Failed to create synchronization gadgets: fence", ExceptionType::INIT_PIPELINE);
        }
    }

    void VkResourceManager::DrawFrame()
    {
        vkWaitForFences(this->device, 1, &this->inFlightFences[this->currentFrameIndex], VK_TRUE, UINT64_MAX);

        UInt imageIndex;
        VkResult result = vkAcquireNextImageKHR(this->device, this->swapchain, UINT64_MAX, this->imageAvailableSemaphores[this->currentFrameIndex], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            this->RecreateSwapchain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            throw Exception("Failed to acquire swapchain image", ExceptionType::UPDATE_RENDER);

        vkResetFences(this->device, 1, &this->inFlightFences[this->currentFrameIndex]);             // Reset here to avoid deadlock

        vkResetCommandBuffer(this->graphicsCommandBuffers[this->currentFrameIndex], 0);
        RecordCommandBuffer(this->graphicsCommandBuffers[this->currentFrameIndex], imageIndex);

        VkSemaphore waitSemaphores[] = { this->imageAvailableSemaphores[this->currentFrameIndex] };
        VkSemaphore signalSemaphores[] = { this->renderFinishedSemaphores[this->currentFrameIndex] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores,
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = &this->graphicsCommandBuffers[this->currentFrameIndex],
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &this->renderFinishedSemaphores[this->currentFrameIndex]
        };

        if (vkQueueSubmit(this->queues[QueueFamilyIndices::GRAPHICS], 1, &submitInfo, this->inFlightFences[this->currentFrameIndex]) != VK_SUCCESS)
            throw Exception("Failed to submit draw command buffer", ExceptionType::UPDATE_RENDER);

        VkSwapchainKHR swapchains[] = { this->swapchain };
        VkPresentInfoKHR presentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signalSemaphores,
            .swapchainCount = 1,
            .pSwapchains = swapchains,
            .pImageIndices = &imageIndex,
            .pResults = nullptr
        };

        result = vkQueuePresentKHR(this->queues[QueueFamilyIndices::PRESENT], &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || this->frameBufferResized)
        {
            this->RecreateSwapchain();
            this->frameBufferResized = false;
        }
        else if (result != VK_SUCCESS)
            throw Exception("Failed to present swapchain image", ExceptionType::UPDATE_RENDER);

        this->currentFrameIndex = (this->currentFrameIndex + 1) % VkResourceManager::maxFramesInFlight;
    }

    void VkResourceManager::CleanUpSwapchain()
    {
        for (const auto& view : swapchainImageViews)
            vkDestroyImageView(this->device, view, nullptr);
        for (const auto& framebuffer : swapchainFrameBuffers)
            vkDestroyFramebuffer(this->device, framebuffer, nullptr);
        vkDestroySwapchainKHR(this->device, this->swapchain, nullptr);
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
        else
            ATR_PRINT_VERBOSE(String("[Validation Layer] ") + pCallbackData->pMessage)

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

        ATR_LOG_SECTION("Vulkan Available Extensions: (" + std::to_string(extensions.size()) + ")")
        for (const auto& extension : extensions)
            ATR_PRINT_VERBOSE(String("- ") + extension.extensionName)

        ATR_LOG_SECTION("Vulkan Required Extensions: (" + std::to_string(instanceExtensions.size()) + ")")
        for (const auto& extension : instanceExtensions)
            ATR_PRINT_VERBOSE(String("- ") + extension)

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

            ATR_LOG_SECTION("Vulkan Available Layers: (" + std::to_string(validatedLayerCount) + ")")
            for (const auto& layer : availableValidatedLayers)
                ATR_PRINT_VERBOSE(String("- ") + layer.layerName)

            ATR_LOG_SECTION("Vulkan Required Layers: (" + std::to_string(this->validationLayers.size()) + ")")
            for (const auto& layer : this->validationLayers)
                ATR_PRINT_VERBOSE(String("- ") + layer)

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

    void VkResourceManager::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
    {
        VkBufferCreateInfo bufferInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        if (vkCreateBuffer(this->device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
            throw Exception("Failed to create buffer", ExceptionType::INIT_PIPELINE);

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(this->device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = this->FindMemoryType(memRequirements.memoryTypeBits, properties)
        };

        if (vkAllocateMemory(this->device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
            throw Exception("Failed to allocate buffer memory", ExceptionType::INIT_PIPELINE);

        vkBindBufferMemory(this->device, buffer, bufferMemory, 0);
    }

    void VkResourceManager::RetrieveSwapChainImages()
    {
        UInt imageCount = 0;
        vkGetSwapchainImagesKHR(this->device, this->swapchain, &imageCount, nullptr);

        this->swapchainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(this->device, this->swapchain, &imageCount, this->swapchainImages.data());

        ATR_PRINT_VERBOSE("Retrieved " + std::to_string(imageCount) + " images in total in the swapchain.")
    }

    void VkResourceManager::RecordCommandBuffer(VkCommandBuffer commandBuffer, UInt imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = 0,
            .pInheritanceInfo = nullptr
        };

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
            throw Exception("Failed to begin recording command buffer", ExceptionType::INIT_PIPELINE);

        VkRenderPassBeginInfo renderPassInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = this->renderPass,
            .framebuffer = this->swapchainFrameBuffers[imageIndex],
            .renderArea = {
                .offset = { 0, 0 },
                .extent = this->swapChainConfig.extent
            },
            .clearValueCount = 1,
            .pClearValues = &VkResourceManager::defaultClearColor
        };

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphicsPipeline);

            VkViewport viewport;
            viewport.x = 0;
            viewport.y = 0;
            viewport.width = (Float)this->swapChainConfig.extent.width;
            viewport.height = (Float)this->swapChainConfig.extent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(this->graphicsCommandBuffers[this->currentFrameIndex], 0, 1, &viewport);

            VkRect2D scissor;
            scissor.offset = { 0, 0 };
            scissor.extent = this->swapChainConfig.extent;
            vkCmdSetScissor(this->graphicsCommandBuffers[this->currentFrameIndex], 0, 1, &scissor);

            VkBuffer vertexBuffers[] = { this->vertexBuffer };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdDraw(commandBuffer, static_cast<UInt>(VkResourceManager::vertices.size()), 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
            throw Exception("Failed to record command buffer", ExceptionType::INIT_PIPELINE);
    }

    UInt VkResourceManager::FindMemoryType(UInt typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(this->physicalDevice, &memProperties);

        for (UInt i = 0; i != memProperties.memoryTypeCount; ++i)
            if (
                typeFilter & (1 << i) &&                                                    // Suitable for the buffer
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties     // Suitable for data
            )
                return i;

        throw Exception("Failed to find suitable memory type", ExceptionType::UPDATE_MEMORY);
    }

    void VkResourceManager::RecreateSwapchain()
    {
        // If window is minimized, pause update
        int width = 0, height = 0;
        glfwGetFramebufferSize(this->window, &width, &height);
        while (width == 0 || height == 0)
        {
            // TODO get rid of busy waiting here
            glfwGetFramebufferSize(this->window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(this->device);

        CleanUpSwapchain();

        CreateSwapchain();
        CreateImageViews();
        CreateFrameBuffers();
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

    void VkResourceManager::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<VkResourceManager*>(glfwGetWindowUserPointer(window));
        ATR_LOG_VERBOSE("Window Resizing... new width: " << width << ", height: " << height)
        app->frameBufferResized = true;
    }
}