#pragma once
#include "atrfwd.h"

#include "Config.h"
#include "VkResources.h"

namespace ATR
{
	class Renderer
	{
	public:
		Renderer(const Config&);

		void Run();

		// Major Components
		void InitWindow();
		void InitVulkan();				// Vulkan helpers are all in class `VkResources`
		void Update();
		void Cleanup();

	private:
		GLFWwindow* window;

		VkResourceManager vkResources;

		const UInt width;
		const UInt height;
	};
}
