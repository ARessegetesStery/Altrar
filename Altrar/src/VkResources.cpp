#include "atrpch.h"

#include "VkResources.h"

namespace ATR
{
    void VkResourceManager::AbsorbConfigs(const Config& config)
    {
        this->verbose = config.verbose;
        for (const String& layerName : config.validationLayers)
            this->validationLayerNames.push_back(layerName.c_str());
    }

    void VkResourceManager::CreateInstance()
    {
        UInt extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        UInt requiredExtensionCount = 0;
        const char** reqExtensions = glfwGetRequiredInstanceExtensions(&requiredExtensionCount);

        if (this->verbose)
        {
            ATR_LOG_SECTION("Vulkan Available Extensions: (" + std::to_string(extensionCount) + ")")
                for (const auto& extension : extensions)
                    ATR_PRINT(String("- ") + extension.extensionName)

            ATR_LOG_SECTION("Vulkan Required Extensions: (" + std::to_string(requiredExtensionCount) + ")")
                for (UInt i = 0; i != requiredExtensionCount; ++i)
                    ATR_PRINT(String("- ") + reqExtensions[i])
        }

        // Check if all required extensions are available
        for (UInt i = 0; i != requiredExtensionCount; ++i)
            if (std::find_if(extensions.begin(), extensions.end(), 
                [&](const VkExtensionProperties& ext) { return strcmp(ext.extensionName, reqExtensions[i]) == 0; }
            ) == extensions.cend())
                throw Exception("Required extension not found: " + String(reqExtensions[i]), ExceptionType::INIT_VULKAN);
    
#if defined ATR_ENABLE_VALIDATION
        UInt validatedLayerCount = 0;
        vkEnumerateInstanceLayerProperties(&validatedLayerCount, nullptr);

        std::vector<VkLayerProperties> validatedLayers(validatedLayerCount);
        vkEnumerateInstanceLayerProperties(&validatedLayerCount, validatedLayers.data());

        if (this->verbose)
        {
            ATR_LOG_SECTION("Vulkan Available Layers: (" + std::to_string(validatedLayerCount) + ")")
            for (const auto& layer : validatedLayers)
                ATR_PRINT(String("- ") + layer.layerName)

            ATR_LOG_SECTION("Vulkan Required Layers: (" + std::to_string(this->validationLayerNames.size()) + ")")
            for (const auto& layer : this->validationLayerNames)
                ATR_PRINT(String("- ") + layer)
        }

        // Check if all validation layers are available
        for (auto& layerName : this->validationLayerNames)
            if (std::find_if(validatedLayers.begin(), validatedLayers.end(), 
                [&](const VkLayerProperties& layer) { return strcmp(layer.layerName, layerName) == 0; }
            ) == validatedLayers.cend())
                throw Exception("Validation layer " + String(layerName)  + " not found", ExceptionType::INIT_VULKAN);
#endif

        VkApplicationInfo appInfo =
        {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Altrar",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "No Engine",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_0
        };

        UInt glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        VkInstanceCreateInfo createInfo =
        {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
#if defined ATR_ENABLE_VALIDATION
            .enabledLayerCount = 1,
            .ppEnabledLayerNames = this->validationLayerNames.data(),
#else
            .enabledLayerCount = 0,
#endif 
            .enabledExtensionCount = glfwExtensionCount,
            .ppEnabledExtensionNames = glfwExtensions,

        };

        if (vkCreateInstance(&createInfo, nullptr, &this->instance) != VK_SUCCESS)
            throw Exception("Failed to create instance", ExceptionType::INIT_VULKAN);
    }

    void VkResourceManager::CleanUp()
    {
        vkDestroyInstance(this->instance, nullptr);
    }
}