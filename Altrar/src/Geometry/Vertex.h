#pragma once

#include "atrfwd.h"

namespace ATR
{ 
    struct Vertex
    {
    public:
        Vertex(Vec3 pos, Vec3 normal, Vec3 color) : pos(pos), normal(normal), color(color) {  }

        Vec3 pos;
        Vec3 normal;
        Vec3 color;

        static VkVertexInputBindingDescription bindingDescription;
        static std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions;
    };

    bool operator==(const Vertex& lhs, const Vertex& rhs);
}
