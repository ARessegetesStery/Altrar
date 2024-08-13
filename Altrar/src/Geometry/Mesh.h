#pragma once

#include "atrfwd.h"

#include "Vertex.h"

namespace ATR
{
    class Mesh
    {
    public:
        void AddTriangle(std::array<Vertex, 3> vertices);

        inline const std::vector<UInt>& GetIndices() const { return this->indices; }
        inline const std::vector<Vertex>& GetVertices() const { return this->vertices; }

    private:
        std::vector<UInt> indices;
        std::vector<Vertex> vertices;
    };

}
