#include "atrpch.h"

#include "VkResources.h"

namespace ATR
{
    void VkResourceManager::AbsorbConfigs(const Config& config)
    {
        VkResourceManager::verbose = config.verbose;
        for (const String& layerName : config.validationLayers)
            this->validationLayers.push_back(layerName.c_str());
    }

    void VkResourceManager::Init(GLFWwindow* window)
    {
        this->window = window;

        this->CreateInstance();
        this->SetupDebugMessenger();
        this->CreateSurface();
        this->SelectPhysicalDevice();
        this->CreateLogicalDevice();
    }

    void VkResourceManager::CleanUp()
    {
        if (enabledValidation)
            this->DestroyDebugUtilsMessengerEXT(this->instance, this->debugMessenger, nullptr);

        vkDestroyDevice(this->device, nullptr);
        vkDestroyInstance(this->instance, nullptr);
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
            .enabledExtensionCount = static_cast<UInt>(this->requiredExtensions.size()),
            .ppEnabledExtensionNames = this->requiredExtensions.data()
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
            .enabledExtensionCount = 0,
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
        this->requiredExtensions.clear();

        UInt extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        UInt requiredExtensionCount = 0;
        const char** requiredExtensionNames = glfwGetRequiredInstanceExtensions(&requiredExtensionCount);

        for (UInt i = 0; i != requiredExtensionCount; ++i)
            this->requiredExtensions.push_back(requiredExtensionNames[i]);

        if (enabledValidation)
            requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        if (VkResourceManager::verbose)
        {
            ATR_LOG_SECTION("Vulkan Available Extensions: (" + std::to_string(extensions.size()) + ")")
            for (const auto& extension : extensions)
                ATR_PRINT(String("- ") + extension.extensionName)

            ATR_LOG_SECTION("Vulkan Required Extensions: (" + std::to_string(requiredExtensions.size()) + ")")
            for (const auto& extension : requiredExtensions)
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

        if (this->queueIndices.Complete())
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
}