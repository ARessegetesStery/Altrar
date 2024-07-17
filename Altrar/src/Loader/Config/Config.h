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
                layerStr += layer + "\n" + Format::indent;
            layerStr = layerStr.substr(0, layerStr.size() - 4);

            return os <<
                Format::smallIndent << "Width: " << config.width << ", " <<
                Format::smallIndent << "Height: " << config.height << "\n" <<
                std::boolalpha <<   // print bools as true/false
                Format::smallIndent << "Verbose: " << config.verbose << "\n" <<
                Format::smallIndent << "Enable Validation: " << config.enableValidation << "\n" <<
                Format::smallIndent << "Validation Layers: \n" << Format::indent << layerStr;
        }
    };

}