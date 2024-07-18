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

        Config();
        Config(const Config&) = default;

        friend std::ostream& operator<< (std::ostream& os, Config const& config)
        {
            String layerStr = "";
            for (const auto& layer : config.validationLayers)
                layerStr += layer + "\n" + Format::subitem;
            layerStr = layerStr.substr(0, layerStr.size() - 4);

            return os <<
                Format::item << "Width: " << config.width << ", " << "Height: " << config.height << "\n" <<
                std::boolalpha <<   // print bools as true/false
                Format::item << "Verbose: " << config.verbose << "\n" <<
                Format::item << "Enable Validation: " << config.enableValidation << "\n" <<
                Format::item << "Validation Layers: \n" << Format::subitem << layerStr;
        }
    };

}