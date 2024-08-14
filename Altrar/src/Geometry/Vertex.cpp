#include "atrpch.h"

#include "Vertex.h"

namespace ATR
{
    VkVertexInputBindingDescription Vertex::bindingDescription = {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    std::array<VkVertexInputAttributeDescription, 3> Vertex::attributeDescriptions = {
        VkVertexInputAttributeDescription{
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, pos)
        },
        VkVertexInputAttributeDescription{
            .location = 1,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, normal)
        },
        VkVertexInputAttributeDescription{
            .location = 2,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, color)
        }
    };

    bool operator==(const Vertex& v1, const Vertex& v2)
    {
        return v1.pos == v2.pos && v1.color == v2.color;
    }
}
