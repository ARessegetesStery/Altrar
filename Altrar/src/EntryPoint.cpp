#include "atrpch.h"

#include "Loader/Config/Config.h"
#include "Renderer.h"

int main(int argc, char** argv)
{
    ATR::Config config;
    ATR::Renderer renderer(std::move(config));
    renderer.Run();

    return 0;
}
