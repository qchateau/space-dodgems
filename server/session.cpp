#include "session.h"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace si {

session::session(tcp::socket&& socket) : ws_(std::move(socket))
{
    spdlog::info("new session 0x{:p}", static_cast<void*>(this));
}

session::~session()
{
    spdlog::info("closing session 0x{:p}", static_cast<void*>(this));
}

void session::run()
{
    net::co_spawn(
        ws_.get_executor(),
        [self = shared_from_this()]() -> net::awaitable<void> {
            co_await self->on_run();
        },
        net::detached);
}

net::awaitable<void> session::on_run()
{
    // Set suggested timeout settings for the websocket
    ws_.set_option(
        websocket::stream_base::timeout::suggested(beast::role_type::server));

    // Accept the websocket handshake
    co_await ws_.async_accept(net::use_awaitable);

    const int w{800};
    const int h{800};
    int x{w / 2};
    int y{h / 2};
    int xinc = 2;
    int yinc = 3;

    auto executor = co_await net::this_coro::executor;
    net::steady_timer timer(executor);

    while (true) {
        x = (x + xinc) % w;
        y = (y + yinc) % h;

        auto msg = nlohmann::json::object({{"x", x}, {"y", y}});
        auto serialized = msg.dump();
        co_await ws_.async_write(net::buffer(serialized), net::use_awaitable);

        timer.expires_after(std::chrono::milliseconds{10});
        co_await timer.async_wait(net::use_awaitable);
    }
}

} // si