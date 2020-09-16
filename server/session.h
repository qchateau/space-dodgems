#pragma once

#include <memory>
#include <vector>

#include <boost/beast.hpp>
#include <nlohmann/json.hpp>

#include "config.h"
#include "player.h"

namespace sd {

class session_t : public std::enable_shared_from_this<session_t> {
public:
    session_t(std::shared_ptr<world_t> world, tcp::socket&& socket);
    ~session_t();

    session_t(const session_t&) = delete;
    session_t(session_t&&) = delete;

    session_t& operator=(const session_t&) = delete;
    session_t& operator=(session_t&&) = delete;

    void run();
    tcp::endpoint remote_endpoint() const { return remote_endpoint_; }

private:
    net::awaitable<void> do_run();
    net::awaitable<void> read_loop();
    net::awaitable<void> write_loop();
    net::awaitable<void> keepalive();
    void cleanup();

    void handle_command(const nlohmann::json& command);
    void handle_input(const nlohmann::json& input);

    std::shared_ptr<world_t> world_;
    player_handle_t player_;
    websocket::stream<beast::tcp_stream> ws_;
    const tcp::endpoint remote_endpoint_;
    net::steady_timer timer_;
};

} // sd