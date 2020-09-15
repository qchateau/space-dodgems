#include <iostream>
#include <spdlog/spdlog.h>

#include "listener.h"
#include "world.h"

using namespace si;

int main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc != 3) {
        std::cerr << "Usage: server <address> <port>\n"
                  << "Example:\n"
                  << "    server 0.0.0.0 8080\n";
        return EXIT_FAILURE;
    }

    auto const address = net::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));

    // The io_context is required for all I/O
    net::io_context ioc{1};

    auto world = std::make_shared<world_t>(ioc);
    std::make_shared<listener_t>(ioc, *world, tcp::endpoint{address, port})->run();
    world->run();

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([&](const beast::error_code&, int) { ioc.stop(); });

    ioc.run();

    return EXIT_SUCCESS;
}