#include "engine/startup.hpp"

#include <string_view>

namespace Zenith
{
    ApplicationConfig parseApplicationConfig(int argc, char **argv)
    {
        ApplicationConfig config{};

        for (int i = 1; i < argc; ++i)
        {
            const std::string_view arg{argv[i] ? argv[i] : ""};

            if (arg == "--no-debug")
            {
                config.debug.enabled = false;
            }
            else if (arg == "--debug")
            {
                config.debug.enabled = true;
            }
            else if (arg == "--no-debug-text")
            {
                config.debug.bgfxText = false;
            }
            else if (arg == "--no-debug-stats")
            {
                config.debug.bgfxStats = false;
            }
            else if (arg == "--debug-text")
            {
                config.debug.bgfxText = true;
            }
            else if (arg == "--debug-stats")
            {
                config.debug.bgfxStats = true;
            }
        }

        return config;
    }

} // namespace Zenith
