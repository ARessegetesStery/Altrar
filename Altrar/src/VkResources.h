#pragma once
#include "atrpch.h"


namespace ATR
{
    class VkResourceManager
    {
    public:
        VkResourceManager() = default;

        void CreateInstance();

    private:
        VkInstance instance;
    };

}