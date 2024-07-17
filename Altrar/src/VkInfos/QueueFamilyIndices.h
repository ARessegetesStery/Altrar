#pragma once
#include "atrfwd.h"

#include <array>

namespace ATR
{
    struct QueueFamilyIndices
    {
        static inline const size_t GRAPHICS = 0;        // graphic operations
        static inline const size_t PRESENT = 1;         // surface presentation support

        static inline const size_t COUNT = 2;

        std::array<std::optional<UInt>, QueueFamilyIndices::COUNT> indices;

        QueueFamilyIndices() 
        {
            for (auto& index : indices)
                index = std::nullopt;
        }

        Bool Complete()
        {
            for (auto& index : indices)
                if (index == std::nullopt)
                    return false;

            return true;
        }

        friend std::ostream& operator<< (std::ostream& os, QueueFamilyIndices const& indices)
        {
            return os << 
                Format::smallIndent << "Graphics Queue Index: " << indices.indices[GRAPHICS].value() << "\n" <<
                Format::smallIndent << "Present Queue Index: " << indices.indices[PRESENT].value();
        }
    };
}