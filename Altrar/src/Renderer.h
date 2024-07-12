#pragma once
#include "atrfwd.h"

#include "Config.h"

namespace ATR
{
	class Renderer
	{
	public:
		Renderer(const Config&);

		void Run();

		void InitWindow();
		void InitVulkan();
		void Update();
		void Cleanup();

	private:
		GLFWwindow* window;

		const UInt width;
		const UInt height;
	};
}
