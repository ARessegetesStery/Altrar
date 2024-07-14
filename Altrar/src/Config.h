#pragma once
#include "atrfwd.h"

namespace ATR
{
    struct Config
    {
        UInt width, height;
        Bool verbose;

        Config() : 
            width(800), height(600),
            verbose(true)
        {   }

        Config(const Config&) = default;

        friend std::ostream& operator<< (std::ostream& os, Config const& config)
        {
            return os << 
                "Width: " << config.width << ", " <<
                "Height: " << config.height << "\n" <<
                "Verbose: " << (config.verbose == true ? "true" : "false");
        }
    };

}