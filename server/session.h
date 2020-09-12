#pragma once

#include <memory>
#include <vector>

#include <boost/beast.hpp>

#include "config.h"

namespace si {

// Echoes back all received WebSocket messages
class session : public std::enable_shared_from_this<session> {
public:
    explicit session(tcp::socket&& socket);
    ~session();
    void run();
    net::awaitable<void> on_run();

private:
    websocket::stream<beast::tcp_stream> ws_;
};

} // si