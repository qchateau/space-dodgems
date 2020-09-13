#pragma once

#include <memory>
#include <vector>

#include <boost/beast.hpp>

#include "config.h"
#include "world.h"

namespace si {

class session : public std::enable_shared_from_this<session> {
public:
    session(world& world, tcp::socket&& socket);
    ~session();

    session(const session&) = delete;
    session(session&&) = delete;

    session& operator=(const session&) = delete;
    session& operator=(session&&) = delete;

    void run();

private:
    net::awaitable<void> do_run();
    net::awaitable<void> read_loop();
    net::awaitable<void> write_loop();

    void handle_key(const std::string& key);

    world& world_;
    player::handle player_;
    websocket::stream<beast::tcp_stream> ws_;
};

} // si