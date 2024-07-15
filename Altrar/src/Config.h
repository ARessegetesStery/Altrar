#pragma once
#include "atrfwd.h"

namespace ATR
{
    struct Config
    {
        UInt width, height;
        Bool enableValidation;
        Bool verbose;
        std::vector<String> validationLayers;

        Config() : 
            width(800), height(600),
            enableValidation(true),
            verbose(true),
            validationLayers({"VK_LAYER_KHRONOS_validation"})
        {   }

        Config(const Config&) = default;

        friend std::ostream& operator<< (std::ostream& os, Config const& config)
        {
            String layerStr;
            String indent = "    ";
            for (const auto& layer : config.validationLayers)
                layerStr += layer + ", \n" + indent;
            layerStr = layerStr.substr(0, layerStr.size() - 4);

            return os <<
                "Width: " << config.width << ", " <<
                "Height: " << config.height << "\n" <<
                "Verbose: " << (config.verbose == true ? "true" : "false") << "\n" <<
                "Validation Layers: \n" << indent << layerStr;
        }
    };

}