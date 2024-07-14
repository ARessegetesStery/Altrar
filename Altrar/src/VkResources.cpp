#include "atrpch.h"

#include "VkResources.h"

namespace ATR
{
    void VkResourceManager::CheckExtensions(Bool print)
    {
        UInt extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        UInt requiredExtensionCount = 0;
        const char** reqExtensions = glfwGetRequiredInstanceExtensions(&requiredExtensionCount);

        if (print)
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
    }

    void VkResourceManager::CreateInstance()
    {
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
            .enabledExtensionCount = glfwExtensionCount,
            .ppEnabledExtensionNames = glfwExtensions
        };

        if (vkCreateInstance(&createInfo, nullptr, &this->instance) != VK_SUCCESS)
            throw Exception("Failed to create instance", ExceptionType::INIT_VULKAN);
    }

    void VkResourceManager::CleanUp()
    {
        vkDestroyInstance(this->instance, nullptr);
    }
}