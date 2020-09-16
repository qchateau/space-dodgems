#pragma once

#include <memory>
#include <string>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <spdlog/spdlog.h>

#include "config.h"

namespace de {

class listener_t : public std::enable_shared_from_this<listener_t> {
public:
    listener_t(
        net::io_context& ioc,
        std::vector<std::shared_ptr<world_t>> worlds,
        tcp::endpoint endpoint);

    void run();

private:
    net::awaitable<void> on_run();

    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<world_t>> worlds_;
};

} // de