#pragma once

#include "atrfwd.h"

namespace ATR
{ 
    struct Vertex
    {
    public:
        Vertex(Vec3 pos, Vec3 color) : pos(pos), color(color) {  }

        Vec3 pos;
        Vec3 color;

        static VkVertexInputBindingDescription bindingDescription;
        static std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions;
    };

    bool operator==(const Vertex& lhs, const Vertex& rhs);
}
