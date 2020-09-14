#pragma once

#include <memory>
#include <vector>

#include <boost/beast.hpp>

#include "config.h"
#include "world.h"

namespace si {

class session_t : public std::enable_shared_from_this<session_t> {
public:
    session_t(world_t& world, tcp::socket&& socket);
    ~session_t();

    session_t(const session_t&) = delete;
    session_t(session_t&&) = delete;

    session_t& operator=(const session_t&) = delete;
    session_t& operator=(session_t&&) = delete;

    void run();

private:
    net::awaitable<void> do_run();
    net::awaitable<void> read_loop();
    net::awaitable<void> write_loop();

    void handle_key(const std::string& key);

    world_t& world_;
    player_t::handle_t player_;
    websocket::stream<beast::tcp_stream> ws_;
};

} // si