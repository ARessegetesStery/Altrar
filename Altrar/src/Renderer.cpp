#include "atrpch.h"
#include "Renderer.h"

namespace ATR
{

    Renderer::Renderer(Config&& config) : 
        window(nullptr),
        config(std::move(config))
    {  }

    void Renderer::Run()
    {
        try
        {
            InitRenderer();
            InitWindow();
            InitVulkan();
            Update();
            Cleanup();
        }
        catch(const Exception& e)
        {
            ATR_ERROR(e.What())
        }
    }

    void Renderer::InitRenderer()
    {
        ATR_LOG_PART("Initializing Renderer");
        ATR_LOG(this->config);
        this->vkResources.AbsorbConfigs(this->config);
    }

    void Renderer::InitWindow()
    {
        ATR_LOG_PART("Initializing Window");
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(this->config.width, this->config.height, "Altrar", nullptr, nullptr);
        if (!window)
            throw Exception("Failed to create window", ExceptionType::INIT_GLFW);
    }

    void Renderer::InitVulkan()
    {
        ATR_LOG_PART("Initializing Vulkan");
        this->vkResources.Init();
    }

    void Renderer::Update()
    {
        while(!glfwWindowShouldClose(window))
            glfwPollEvents();
    }

    void Renderer::Cleanup()
    {
        this->vkResources.CleanUp();

        glfwDestroyWindow(window);
        glfwTerminate();
    }
}
