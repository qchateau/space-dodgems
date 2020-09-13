#include "world.h"

namespace si {

world::world(net::io_context& ioc) : ioc_{ioc} {}

typename player::handle world::registerPlayer()
{
    auto& new_player = players_.emplace_back(*this);
    spdlog::info("registering new player");
    return {&new_player, [this](player* p) {
                auto it = find_if(
                    begin(players_), end(players_), [p](const auto& player) {
                        return *p == player;
                    });
                if (it == end(players_)) {
                    spdlog::warn("unregistering unknown player");
                    return;
                }
                spdlog::info("unregistering player");
                players_.erase(it);
            }};
}

void world::run()
{
    net::co_spawn(
        ioc_,
        [self = shared_from_this()]() -> net::awaitable<void> {
            co_await self->on_run();
        },
        net::detached);
}

net::awaitable<void> world::on_run()
{
    const auto dt = std::chrono::milliseconds{17};
    auto executor = co_await net::this_coro::executor;
    net::steady_timer timer{executor};
    timer.expires_from_now(std::chrono::seconds{0});

    while (true) {
        update(dt);
        timer.expires_at(timer.expires_at() + dt);
        co_await timer.async_wait(net::use_awaitable);
    }
}

void world::update(std::chrono::nanoseconds dt)
{
    for (player& p : players_) {
        p.update_pos(dt);
    }
}

} // si