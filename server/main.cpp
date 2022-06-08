#include <iostream>
#include <spdlog/spdlog.h>

#include "listener.h"
#include "world.h"

using namespace sd;

constexpr auto addr_envvar = "ADDR";
constexpr auto port_envvar = "PORT";
constexpr auto nworlds_envvar = "NWORLDS";

int main(int /*argc*/, char* /*argv*/[])
{
    const auto* mb_address = std::getenv(addr_envvar);
    if (mb_address == nullptr) {
        std::cerr << "Environment variable " << addr_envvar << " is not defined"
                  << std::endl;
        return EXIT_FAILURE;
    }
    const auto* mb_port = std::getenv(port_envvar);
    if (mb_address == nullptr) {
        std::cerr << "Environment variable " << port_envvar << " is not defined"
                  << std::endl;
        return EXIT_FAILURE;
    }
    const auto* mb_nworlds = std::getenv(nworlds_envvar);
    if (mb_address == nullptr) {
        std::cerr << "Environment variable " << nworlds_envvar
                  << " is not defined" << std::endl;
        return EXIT_FAILURE;
    }

    const auto address = net::ip::make_address(mb_address);
    const auto port = static_cast<unsigned short>(std::atoi(mb_port));
    const auto nworlds = std::atoi(mb_nworlds);

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
