#include <iostream>
#include <spdlog/spdlog.h>

#include "listener.h"
#include "world.h"

using namespace sd;

int main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc != 4) {
        std::cerr << "Usage: server <address> <port> <nworlds>\n"
                  << "Example:\n"
                  << "    server 0.0.0.0 8080 1\n";
        return EXIT_FAILURE;
    }

    const auto address = net::ip::make_address(argv[1]);
    const auto port = static_cast<unsigned short>(std::atoi(argv[2]));
    const auto nworlds = std::atoi(argv[3]);

    // The io_context is required for all I/O
    net::io_context ioc{1};

    std::vector<std::shared_ptr<world_t>> worlds;
    for (int i = 0; i < nworlds; ++i) {
        worlds.emplace_back(std::make_shared<world_t>(ioc));
        worlds.back()->run();
    }
    std::make_shared<listener_t>(ioc, std::move(worlds), tcp::endpoint{address, port})
        ->run();

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([&](const beast::error_code&, int) { ioc.stop(); });

    ioc.run();

    return EXIT_SUCCESS;
}