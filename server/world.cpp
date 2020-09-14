#include "world.h"

namespace si {

world::world(net::io_context& ioc) : ioc_{ioc} {}

typename player::handle world::register_player()
{
    auto& new_player = players_.emplace_back(*this);
    set_initial_player_pos(new_player);

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

void world::set_initial_player_pos(player& p)
{
    std::uniform_real_distribution<> rnd(0.1, 0.9);
    p.set_pos(rnd(rnd_gen_), rnd(rnd_gen_));
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
            auto lifetime =
                std::chrono::duration_cast<std::chrono::duration<double>>(
                    p.lifetime())
                    .count();
            return nlohmann::json({
                {"x", p.state().x},
                {"y", p.state().y},
                {"dx", p.state().dx},
                {"dy", p.state().dy},
                {"ddx", p.state().ddx},
                {"ddy", p.state().ddy},
                {"width", p.state().width},
                {"height", p.state().height},
                {"lifetime", lifetime},
                {"is_me", is_me},
                {"alive", p.alive()},
            });
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
        if (!p.is_in_world()) {
            p.kill();
        }
    }

    for (auto player_it = begin(players_); player_it != end(players_);
         ++player_it) {
        for (auto other_it = ++decltype(player_it)(player_it);
             other_it != end(players_);
             ++other_it) {
            if (!player_it->collides(*other_it)) {
                continue;
            }

            auto& killed = player_it->state().l1_speed()
                                   <= other_it->state().l1_speed()
                               ? *player_it
                               : *other_it;
            killed.kill();
        }
    }
}

} // si