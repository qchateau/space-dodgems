#include "world.h"

namespace si {

world_t::world_t(net::io_context& ioc) : ioc_{ioc} {}

int world_t::real_players() const
{
    int real_players = 0;
    for (const player_t& p : players_) {
        if (!p.fake()) {
            ++real_players;
        }
    }
    return real_players;
}

int world_t::available_places() const
{
    return max_players - real_players();
}

typename player_t::handle_t world_t::register_player()
{
    return register_player(false);
}

typename player_t::handle_t world_t::register_player(bool fake)
{
    auto& new_player = players_.emplace_back(*this, fake);
    set_initial_player_pos(new_player);

    spdlog::debug("registered player {}", new_player.id());

    adjust_players();
    return {
        &new_player,
        [self = shared_from_this()](player_t* p) { self->unregister_player(*p); },
    };
}

void world_t::unregister_player(const player_t& p)
{
    auto it = find_if(begin(players_), end(players_), [&](const auto& player) {
        return p == player;
    });
    if (it == end(players_)) {
        spdlog::warn("unregistered unknown player {}", p.id());
        return;
    }
    players_.erase(it);
    spdlog::debug("unregistered player {}", p.id());

    adjust_players();
}

void world_t::adjust_players()
{
    if (real_players() == 0) {
        if (!fake_players_.empty()) {
            spdlog::info("no more players, removing all fake players");
            fake_players_.clear();
        }
        return;
    }

    int missing = max_players - players_.size();
    if (missing > 0) {
        spdlog::debug("adding {} fake players", missing);
        for (int i = 0; i < missing; ++i) {
            fake_players_.emplace_back(register_player(true));
        }
    }
    else if (missing < 0) {
        int nr_to_remove = std::min<int>(-missing, fake_players_.size());
        spdlog::debug("removing {} fake players", nr_to_remove);
        for (int i = 0; i < nr_to_remove; ++i) {
            fake_players_.pop_back();
        }
    }
}

void world_t::run()
{
    net::co_spawn(
        ioc_,
        [self = shared_from_this()]() -> net::awaitable<void> {
            co_await self->on_run();
        },
        net::detached);
}

void world_t::set_initial_player_pos(player_t& p)
{
    std::uniform_real_distribution<> rnd(0.1, 0.9);
    p.set_pos(rnd(rnd_gen_), rnd(rnd_gen_));
}

nlohmann::json world_t::game_state_for_player(const player_t::handle_t& player)
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
                {"score", p.score()},
                {"is_me", is_me},
                {"alive", p.alive()},
            });
        });

    state["game_over"] = game_over;
    return state;
}

net::awaitable<void> world_t::on_run()
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

void world_t::update(std::chrono::nanoseconds dt)
{
    // update player positions
    for (player_t& p : players_) {
        if (p.fake()) {
            update_fake_player_dd(p);
        }
        p.update_pos(dt);
        if (!p.is_in_world()) {
            p.kill();
        }
    }

    // check for collisions
    for (auto player_it = begin(players_); player_it != end(players_);
         ++player_it) {
        for (auto other_it = ++decltype(player_it)(player_it);
             other_it != end(players_);
             ++other_it) {
            if (!player_it->collides(*other_it)) {
                continue;
            }

            if (player_it->speed() > other_it->speed()) {
                player_it->add_score(other_it->score());
                other_it->kill();
            }
            else {
                other_it->add_score(player_it->score());
                player_it->kill();
            }
        }
    }

    // remove killed fake players
    for (auto it = begin(fake_players_); it != end(fake_players_);) {
        if (!(*it)->alive()) {
            it = fake_players_.erase(it);
        }
        else {
            ++it;
        }
    }
    adjust_players();
}

void world_t::update_fake_player_dd(player_t& p)
{
    const auto l1_dist_to = [&p](double x, double y) {
        return std::abs(p.state().x - x) + std::abs(p.state().y - y);
    };

    double closest_x = 0.5;
    double closest_y = 0.5;
    double closest_distance = l1_dist_to(closest_x, closest_y);

    for (const player_t& other : players_) {
        if (p == other) {
            continue;
        }

        if (auto d = l1_dist_to(other.state().x, other.state().y);
            d < closest_distance) {
            closest_x = other.state().x;
            closest_y = other.state().y;
            closest_distance = d;
        }
    }

    constexpr double fake_player_speed_factor = 2;
    const auto dx = closest_x - p.state().x;
    const auto dy = closest_y - p.state().y;
    p.set_dd(
        fake_player_speed_factor * dx * player_t::max_dd,
        fake_player_speed_factor * dy * player_t::max_dd);
}

} // si