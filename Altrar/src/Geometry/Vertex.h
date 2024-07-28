#pragma once

#include "atrfwd.h"

namespace ATR
{ 
    struct Vertex
    {
    public:
        Vec2 pos;
        Vec3 Color;

        static VkVertexInputBindingDescription bindingDescription;
        static std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions;
    };
}
