#pragma once

#include "ATRType.h"

namespace ATR
{
    enum class ExceptionType
    {
        INIT_RENDERER,
        INIT_GLFW, INIT_VULKAN
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
            case ExceptionType::INIT_GLFW:
                typeStr += "[INIT] (GLFW)";
                break;
            case ExceptionType::INIT_VULKAN:
                typeStr += "[INIT] (VULKAN)";
                break;
            }

            return typeStr + " " + this->msg;
        }
    
    private:
        String msg;
        ExceptionType type;
    };

}