#include "atrpch.h"

#include "Config.h"
#include "Renderer.h"

int main(int argc, char** argv)
{
    ATR::Config config;

    ATR::Renderer renderer(config);
    renderer.Run();

    return 0;
}