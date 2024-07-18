#include "atrpch.h"
#include "Renderer.h"

namespace ATR
{

    Renderer::Renderer(Config&& config) : 
        config(std::move(config))
    {  }

    void Renderer::Run()
    {
        try
        {
            InitRenderer();
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
        ATR_LOG("Running with Config: \n" << this->config);
        this->vkResources.AbsorbConfigs(this->config);
    }

    void Renderer::InitVulkan()
    {
        ATR_LOG_PART("Initializing Vulkan");
        this->vkResources.Init();
    }

    void Renderer::Update()
    {
        this->vkResources.Update();
    }

    void Renderer::Cleanup()
    {
        this->vkResources.CleanUp();
    }
}
