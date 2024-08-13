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
        while (!this->vkResources.ShouldClose())
        {
            this->vkResources.UpdateFrame();
            UpdateStats();
        }
    }

    void Renderer::Cleanup()
    {
        this->vkResources.CleanUp();
    }

    void Renderer::UpdateStats()
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        Float duration = std::chrono::duration<Float, std::chrono::seconds::period>(currentTime - this->lastTime).count();
        Float fps = 1.0f / duration;
        this->lastTime = currentTime;
        ATR_LOG_VERBOSE("Rendering Frame " << this->frameCount << " [ FPS: " 
            << std::fixed << std::setfill(' ') << std::right << std::setw(7) << std::setprecision(2) << fps << " ] "
            << this->vkResources.GetUpdateInfo())
        ++frameCount;
    }

    void Renderer::UpdateMesh(const Mesh& mesh)
    {
         this->vkResources.UpdateMesh(mesh);
    }
}
