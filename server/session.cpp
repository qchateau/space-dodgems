#include "session.h"
#include "world.h"

#include <boost/uuid/string_generator.hpp>
#include <spdlog/spdlog.h>

namespace sd {

namespace {

constexpr auto keepalive_period = std::chrono::seconds{60};

bool player_name_is_valid(std::string_view name)
{
    return name.size() >= 3 && name.size() <= 30;
}

}

session_t::session_t(std::shared_ptr<world_t> world, tcp::socket&& socket)
    : world_{std::move(world)}, ws_{std::move(socket)}, timer_{ws_.get_executor()}
{
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

    // Handle client registration
    std::string str_buffer;
    auto buffer = net::dynamic_buffer(str_buffer);
    co_await ws_.async_read(buffer, net::use_awaitable);
    auto registration = nlohmann::json::parse(
        str_buffer)["command"]["register"];
    auto player_id = registration["id"];
    auto player_name = registration["name"];

    if (!player_id.is_string() || !player_name.is_string()) {
        spdlog::warn("client failed to register");
        co_await ws_.async_close(
            beast::websocket::close_reason{"registration error"},
            net::use_awaitable);
        co_return;
    }

    if (!player_name_is_valid(player_name.get<std::string>())) {
        co_await ws_.async_close(
            beast::websocket::close_reason{"invalid name"}, net::use_awaitable);
        co_return;
    }

    std::optional<beast::websocket::close_reason> close_reason;
    try {
        player_ = world_->register_player(
            boost::uuids::string_generator{}(player_id.get<std::string>()),
            player_name.get<std::string>());
    }
    catch (const player_already_registered& exc) {
        close_reason.emplace(exc.what());
    }
    if (close_reason) {
        co_await ws_.async_close(*close_reason, net::use_awaitable);
        co_return;
    }

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
    net::co_spawn(
        executor,
        [self = shared_from_this()]() -> net::awaitable<void> {
            co_await self->keepalive();
        },
        net::detached);
}

net::awaitable<void> session_t::read_loop()
{
    while (ws_.is_open()) {
        std::string str_buffer;
        auto buffer = net::dynamic_buffer(str_buffer);

        try {
            co_await ws_.async_read(buffer, net::use_awaitable);
        }
        catch (const boost::system::system_error&) {
            break;
        }

        auto msg = nlohmann::json::parse(str_buffer);
        if (msg.contains("command")) {
            handle_command(msg["command"]);
        }
        if (msg.contains("input")) {
            handle_input(msg["input"]);
        }

        timer_.cancel();
    }

    // handle socket errors/close in read_loop
    // because it's where it's easier
    cleanup();
}

net::awaitable<void> session_t::write_loop()
{
    auto executor = co_await net::this_coro::executor;
    net::steady_timer timer(executor);
    timer.expires_from_now(std::chrono::seconds{0});

    while (ws_.is_open()) {
        auto msg = world_->game_state_for_player(player_).dump();
        co_await ws_.async_write(net::buffer(msg), net::use_awaitable);

        timer.expires_at(timer.expires_at() + world_t::refresh_dt);
        co_await timer.async_wait(net::use_awaitable);
    }
}

net::awaitable<void> session_t::keepalive()
{
    while (ws_.is_open()) {
        std::optional<beast::websocket::close_reason> reason;
        try {
            timer_.expires_from_now(keepalive_period);
            co_await timer_.async_wait(net::use_awaitable);
            if (player_->alive()) {
                // still alive despite no command, let this one open
                continue;
            }
            reason.emplace("idle for too long");
        }
        catch (const boost::system::system_error& exc) {
            if (exc.code() == net::error::operation_aborted) {
                continue;
            }
            spdlog::error("unexpected exception: {}", exc.what());
            reason.emplace(exc.what());
        }
        catch (const std::exception& exc) {
            spdlog::error("unexpected exception: {}", exc.what());
            reason.emplace(exc.what());
        }

        if (reason) {
            co_await ws_.async_close(*reason, net::use_awaitable);
            break;
        }
    }
}

void session_t::cleanup()
{
    // most likely the socket is already closed
    // but make close it nonetheless to ensure the
    // session will end asap
    boost::system::error_code ec;
    ws_.close(beast::websocket::close_reason{"session closed"}, ec);

    // cancel the timer so that the keepalive coroutine returns asap
    timer_.cancel();
}

void session_t::handle_command(const nlohmann::json& command)
{
    if (!player_->alive() && command.contains("respawn")) {
        player_->respawn();
    }
}

void session_t::handle_input(const nlohmann::json& input)
{
    if (input.contains("ddx") && input.contains("ddy")) {
        player_->set_dd(input["ddx"].get<double>(), input["ddy"].get<double>());
    }
}

} // sd