#include "atrpch.h"

#include "VkResources.h"

namespace ATR
{
    void VkResourceManager::AbsorbConfigs(const Config& config)
    {
        this->verbose = config.verbose;
        for (const String& layerName : config.validationLayers)
            this->validationLayers.push_back(layerName.c_str());
    }

    void VkResourceManager::Init()
    {
        this->CreateInstance();
        this->SetupDebugMessenger();
    }

    void VkResourceManager::CleanUp()
    {
        //if (enabledValidation)
            //this->DestroyDebugUtilsMessengerEXT(this->instance, this->debugMessenger, nullptr);

        vkDestroyInstance(this->instance, nullptr);
    }

    void VkResourceManager::CreateInstance()
    {
        /// Extensions
        UInt extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        UInt requiredExtensionCount = 0;
        const char** requiredExtensionNames = glfwGetRequiredInstanceExtensions(&requiredExtensionCount);

        std::vector<const char*> requiredExtensions(requiredExtensionNames, requiredExtensionNames + requiredExtensionCount);
        if (enabledValidation)
            requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        if (this->verbose)
        {
            ATR_LOG_SECTION("Vulkan Available Extensions: (" + std::to_string(extensionCount) + ")")
            for (const auto& extension : extensions)
                ATR_PRINT(String("- ") + extension.extensionName)

            ATR_LOG_SECTION("Vulkan Required Extensions: (" + std::to_string(requiredExtensionCount) + ")")
            for (const auto& extension : requiredExtensions)
                ATR_PRINT(String("- ") + extension)
        }

        // Check if all required extensions are available
        for (UInt i = 0; i != requiredExtensionCount; ++i)
            if (std::find_if(extensions.begin(), extensions.end(),
                [&](const VkExtensionProperties& ext) { return strcmp(ext.extensionName, requiredExtensionNames[i]) == 0; }
            ) == extensions.cend())
                throw Exception("Required extension not found: " + String(requiredExtensionNames[i]), ExceptionType::INIT_VULKAN);

        /// Validation Layers 
        if (enabledValidation)
        {
            UInt validatedLayerCount = 0;
            vkEnumerateInstanceLayerProperties(&validatedLayerCount, nullptr);

            std::vector<VkLayerProperties> validatedLayers(validatedLayerCount);
            vkEnumerateInstanceLayerProperties(&validatedLayerCount, validatedLayers.data());

            if (this->verbose)
            {
                ATR_LOG_SECTION("Vulkan Available Layers: (" + std::to_string(validatedLayerCount) + ")")
                for (const auto& layer : validatedLayers)
                    ATR_PRINT(String("- ") + layer.layerName)

                ATR_LOG_SECTION("Vulkan Required Layers: (" + std::to_string(this->validationLayers.size()) + ")")
                for (const auto& layer : this->validationLayers)
                    ATR_PRINT(String("- ") + layer)
            }

            // Check if all validation layers are available
            for (auto& layerName : this->validationLayers)
                if (std::find_if(validatedLayers.begin(), validatedLayers.end(),
                    [&](const VkLayerProperties& layer) { return strcmp(layer.layerName, layerName) == 0; }
                ) == validatedLayers.cend())
                    throw Exception("Validation layer " + String(layerName) + " not found", ExceptionType::INIT_VULKAN);
        }

        VkApplicationInfo appInfo =
        {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Altrar",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "No Engine",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_0
        };

        VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = DebugCallback,
            .pUserData = nullptr
        };

        VkInstanceCreateInfo createInfo =
        {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
            .enabledExtensionCount = static_cast<UInt>(requiredExtensions.size()),
            .ppEnabledExtensionNames = requiredExtensions.data()
        };

        if (enabledValidation)
        {
            createInfo.enabledLayerCount = static_cast<UInt>(this->validationLayers.size());
            createInfo.ppEnabledLayerNames = this->validationLayers.data();

            createInfo.pNext = &debugMessengerCreateInfo;
        }
        else
            createInfo.enabledLayerCount = 0;

        if (vkCreateInstance(&createInfo, nullptr, &this->instance) != VK_SUCCESS)
            throw Exception("Failed to create instance", ExceptionType::INIT_VULKAN);
    }

    void VkResourceManager::SetupDebugMessenger()
    {
        if (!this->enabledValidation)
            return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;
        createInfo.pUserData = nullptr;

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
            throw Exception("Failed to set up debug messenger", ExceptionType::INIT_VULKAN);
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
}