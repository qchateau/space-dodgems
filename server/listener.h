#pragma once

#include <memory>
#include <string>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <spdlog/spdlog.h>

#include "config.h"

namespace si {
// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener> {
public:
    listener(net::io_context& ioc, tcp::endpoint endpoint);

    void run();

private:
    net::awaitable<void> on_run();

    net::io_context& ioc_;
    tcp::acceptor acceptor_;
};

} // si