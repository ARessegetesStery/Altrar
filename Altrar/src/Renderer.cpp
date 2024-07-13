#include "atrpch.h"
#include "Renderer.h"

namespace ATR
{

	Renderer::Renderer(const Config& config) : 
		window(nullptr),
		width(config.width), height(config.height)
	{  }

	void Renderer::Run()
	{
		try
		{
			InitWindow();
			InitVulkan();
			Update();
			Cleanup();
		}
		catch(const Exception& e)
		{
			std::cerr << e.What() << std::endl;
		}
	}

	void Renderer::InitWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(this->width, this->height, "Altrar", nullptr, nullptr);
		if (!window)
			throw Exception("Failed to create window", ExceptionType::INIT_GLFW);
	}

	void Renderer::InitVulkan()
	{
		this->vkResources.CreateInstance();
	}

	void Renderer::Update()
	{
		while(!glfwWindowShouldClose(window))
			glfwPollEvents();
	}

	void Renderer::Cleanup()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}
}
