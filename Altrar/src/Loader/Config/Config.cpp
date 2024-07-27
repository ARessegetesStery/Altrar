#include "atrpch.h"

#include "Config.h"

#include "fkyaml/node.hpp"

namespace ATR
{

#define LOAD_DATA_FROM_YAML(var, node, tag, TYPE) \
    if (node.contains(#tag)) \
        var = node[#tag].get_value<TYPE>(); \
    else \
        throw fkyaml::exception(("missing tag " + std::string(#tag)).c_str());

#define LOAD_DATA_FROM_YAML_NOERROR(var, node, tag, TYPE) \
    if (node.contains(#tag)) \
        var = root[#tag].get_value<TYPE>(); \
    else \
        ATR_PRINT("[INFO] tag \"" + String(#tag) + "\" not specified.");

#define LOAD_DEF_DATA_FROM_YAML(var, node, tag, TYPE) \
    TYPE var; \
    LOAD_DATA_FROM_YAML(var, node, tag, TYPE)

#define LOAD_DEF_DATA_FROM_YAML_NOERROR(var, node, tag, TYPE) \
    TYPE var; \
    LOAD_DATA_FROM_YAML_NOERROR(var, node, tag, TYPE)

#define LOAD_NODE_FROM_YAML(node, root, tag) \
    auto node = root; \
    if (root.contains(#tag)) \
        node = root[#tag]; \
    else \
        throw fkyaml::exception(("missing tag " + std::string(#tag)).c_str());

#define LOAD_NODE_FROM_YAML_NOERROR(node, root, tag) \
    auto node = root; \
    if (root.contains(#tag)) \
        node = root[#tag]; \
    else \
        ATR_PRINT("[INFO] node \"" + String(#tag) + "\" not specified.");

    Config::Config() : 
        width(800), height(600),
        enableValidation(true),
        validationLayers({"VK_LAYER_KHRONOS_validation"})
    {
        const String configPath = "config.yaml";
        std::ifstream ifs(configPath);

        try
        {
            if (!ifs)
            {
                std::string msg = "error opening config file " + configPath;
                throw fkyaml::exception(msg.c_str());
            }
            fkyaml::node root = fkyaml::node::deserialize(ifs);

            LOAD_DATA_FROM_YAML_NOERROR(this->width, root, width, UInt);
            LOAD_DATA_FROM_YAML_NOERROR(this->height, root, height, UInt);
            LOAD_DATA_FROM_YAML_NOERROR(this->enableValidation, root, validation, Bool);
            if (this->enableValidation)
            {
                LOAD_NODE_FROM_YAML_NOERROR(validationLayerNode, root, validation-layers);
                if (validationLayerNode != root)
                    for (const auto& layer : validationLayerNode)
                    {
                        String str = layer.get_value<String>();
                        Bool duplicate = false;
                        for (auto& layer : this->validationLayers)
                        {
                            if (layer == str)
                            {
                                duplicate = true;
                                break;
                            }
                        }
                        if (!duplicate)
                            this->validationLayers.push_back(str);
                    }
            }
        }
        catch (fkyaml::exception& e)
        {
            ATR_PRINT("[WARNING] " + String(e.what()));
            ATR_PRINT("No config specified, using default config.");
        }

    }
}