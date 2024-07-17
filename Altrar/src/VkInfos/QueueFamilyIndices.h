#pragma once
#include "atrfwd.h"

namespace ATR
{
    struct QueueFamilyIndices
    {
        std::optional<UInt> graphicsFamily;

        QueueFamilyIndices() : graphicsFamily(std::nullopt) {}

        Bool Complete()
        {
            return graphicsFamily.has_value();
        }

        friend std::ostream& operator<< (std::ostream& os, QueueFamilyIndices const& indices)
        {
            return os << 
                Format::smallIndent << "Graphics Family: " << indices.graphicsFamily.value();
        }
    };
}