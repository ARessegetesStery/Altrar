#pragma once
#include "atrfwd.h"

namespace ATR
{
    struct Config
    {
        UInt width, height;

        Config() : 
            width(800), height(600)
        {   }
    };

}