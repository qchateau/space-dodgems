#include "world.h"

namespace si {

world::world(net::io_context& ioc) : ioc_{ioc} {}

typename player::handle world::register_player()
{
    auto& new_player = players_.emplace_back(*this);
    spdlog::info("registering player {}", new_player.id());
    return {
        &new_player,
        [self = shared_from_this()](player* p) { self->unregister_player(*p); },
    };
}

void world::unregister_player(const player& p)
{
    auto it = find_if(begin(players_), end(players_), [&](const auto& player) {
        return p == player;
    });
    if (it == end(players_)) {
        spdlog::warn("unregistering unknown player {}", p.id());
        return;
    }
    spdlog::info("unregistering player {}", p.id());
    players_.erase(it);
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

nlohmann::json world::game_state_for_player(const player::handle& player)
{
    nlohmann::json state = {{"players", nlohmann::json::array()}};

    bool game_over = false;
    transform(
        begin(players_),
        end(players_),
        back_inserter(state["players"]),
        [&](const auto& p) {
            bool is_me = p == *player;
            if (is_me && !p.alive()) {
                game_over = true;
            }
            return nlohmann::json(
                {{"x", p.x()},
                 {"y", p.y()},
                 {"is_me", is_me},
                 {"alive", p.alive()}});
        });

    state["game_over"] = game_over;
    return state;
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
        if (!is_in_world(p)) {
            p.kill();
        }
    }
}

bool world::is_in_world(const player& p) const
{
    return 0 <= p.x() && p.x() < 1 && 0 <= p.y() && p.y() < 1;
}

} // si