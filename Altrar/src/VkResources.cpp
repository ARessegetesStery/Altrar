#include "atrpch.h"

#include "VkResources.h"

namespace ATR
{
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