#pragma once

#include "ATRType.h"

namespace ATR
{
    enum class ExceptionType
    {
        INIT_RENDERER,
        INIT_GLFW, INIT_VULKAN,
        INIT_SHADER, INIT_PIPELINE
    };

    class Exception
    {
    public:
        Exception(const String& msg, const ExceptionType type) : msg(msg), type(type) { }
    
        const String& What() const { return this->msg; }
        const ExceptionType& Type() const { return this->type; }
        const String& Msg() const
        {
            String typeStr = "";
            switch (this->type)
            {
            case ExceptionType::INIT_RENDERER:
                typeStr += "[INIT] (Renderer)";
                break;
            case ExceptionType::INIT_GLFW:
                typeStr += "[INIT] (GLFW)";
                break;
            case ExceptionType::INIT_VULKAN:
                typeStr += "[INIT] (Vulkan)";
                break;
            case ExceptionType::INIT_SHADER:
                typeStr += "[INIT] (Shader)";
                break;
            case ExceptionType::INIT_PIPELINE:
                typeStr += "[INIT] (Pipeline)";
                break;
            }

            return typeStr + " " + this->msg;
        }
    
    private:
        String msg;
        ExceptionType type;
    };

}
