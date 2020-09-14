#include "session.h"

#include <spdlog/spdlog.h>

namespace si {

session_t::session_t(world_t& world, tcp::socket&& socket)
    : world_{world}, ws_{std::move(socket)}
{
    spdlog::info("new session {:p}", static_cast<void*>(this));
    player_ = world.register_player();
}

session_t::~session_t()
{
    spdlog::info("closing session {:p}", static_cast<void*>(this));
}

void session_t::run()
{
    net::co_spawn(
        ws_.get_executor(),
        [self = shared_from_this()]() -> net::awaitable<void> {
            co_await self->do_run();
        },
        net::detached);
}

net::awaitable<void> session_t::do_run()
{
    // Set suggested timeout settings for the websocket
    ws_.set_option(
        websocket::stream_base::timeout::suggested(beast::role_type::server));

    // Accept the websocket handshake
    co_await ws_.async_accept(net::use_awaitable);

    // Start the receive and send loops
    auto executor = co_await net::this_coro::executor;
    net::co_spawn(
        executor,
        [self = shared_from_this()]() -> net::awaitable<void> {
            co_await self->read_loop();
        },
        net::detached);
    net::co_spawn(
        executor,
        [self = shared_from_this()]() -> net::awaitable<void> {
            co_await self->write_loop();
        },
        net::detached);
}

net::awaitable<void> session_t::read_loop()
{
    while (true) {
        std::string str_buffer;
        auto buffer = net::dynamic_buffer(str_buffer);
        co_await ws_.async_read(buffer, net::use_awaitable);
        auto msg = nlohmann::json::parse(str_buffer);
        if (msg.contains("command")) {
            handle_command(msg["command"]);
        }
    }
}

net::awaitable<void> session_t::write_loop()
{
    const auto dt = std::chrono::milliseconds{17};
    auto executor = co_await net::this_coro::executor;
    net::steady_timer timer(executor);
    timer.expires_from_now(std::chrono::seconds{0});

    while (true) {
        auto msg = world_.game_state_for_player(player_).dump();
        co_await ws_.async_write(net::buffer(msg), net::use_awaitable);

        timer.expires_at(timer.expires_at() + dt);
        co_await timer.async_wait(net::use_awaitable);
    }
}

void session_t::handle_command(const nlohmann::json& command)
{
    if (command.contains("ddx") && command.contains("ddy")) {
        player_->set_dd(command["ddx"].get<double>(), command["ddy"].get<double>());
    }
    else {
        spdlog::warn("bad command: {}", command.dump());
    }
}

} // si