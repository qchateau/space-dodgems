#include "world.h"
#include "player.h"

#include <array>

#include <boost/uuid/uuid_io.hpp>

namespace sd {

namespace {

std::string get_fake_player_name(
    const std::vector<std::unique_ptr<player_t>>& current_players)
{
    static const auto names = std::array{
        "Rambo",
        "Borg",
        "Chuck Norris",
        "Bruce Lee",
        "Hubert Bonisseur de La Bath",
        "Mickey O'Neil",
        "Luke",
        "Spock",
    };
    int idx = 0;
    for (; idx < names.size() - 1; ++idx) {
        const auto& name = names[idx];
        auto it = find_if(
            begin(current_players), end(current_players), [&](const auto& p) {
                return name == p->name();
            });
        if (it == end(current_players)) {
            break;
        }
    }
    return names[idx];
}

}

world_t::world_t(net::io_context& ioc) : ioc_{ioc} {}

world_t::~world_t() = default;

int world_t::real_players() const
{
    int real_players = 0;
    for (const auto& p : players_) {
        if (!p->fake()) {
            ++real_players;
        }
    }
    return real_players;
}

int world_t::available_places() const
{
    return max_players - real_players();
}

player_handle_t world_t::register_player(
    const player_id_t& player_id,
    std::string_view player_name)
{
    return register_player(player_id, player_name, false);
}

player_handle_t world_t::register_player(
    const player_id_t& player_id,
    std::string_view player_name,
    bool fake)
{
    auto player_it = find_if(begin(players_), end(players_), [&](const auto& p) {
        return p->id() == player_id;
    });
    if (player_it != end(players_)) {
        // player already registered
        throw player_already_registered{};
    }

    auto& new_player = players_.emplace_back(
        std::make_unique<player_t>(*this, player_id, player_name, fake));

    if (!fake) {
        spdlog::info("registered player {}", to_string(new_player->id()));
    }

    net::post(ioc_, [this]() { adjust_players(); });
    return {
        new_player.get(),
        [self = shared_from_this()](player_t* p) { self->unregister_player(*p); },
    };
}

void world_t::unregister_player(const player_t& p)
{
    auto it = find_if(begin(players_), end(players_), [&](const auto& player) {
        return p == *player;
    });
    if (it == end(players_)) {
        spdlog::warn("unregistered unknown player {}", to_string(p.id()));
        return;
    }
    players_.erase(it);

    if (!p.fake()) {
        spdlog::info("unregistered player {}", to_string(p.id()));
    }

    net::post(ioc_, [this]() { adjust_players(); });
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
            fake_players_.emplace_back(register_player(
                uuid_generator_(), get_fake_player_name(players_), true));
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

nlohmann::json world_t::game_state_for_player(const player_handle_t& player)
{
    nlohmann::json state = {
        {"players", nlohmann::json::array()}, {"game_over", false}};

    for (const auto& p_ptr : players_) {
        const auto& p = *p_ptr;
        const bool is_me = p == *player;
        if (is_me && !p.alive()) {
            state["game_over"] = true;
        }

        state["players"].push_back(nlohmann::json({
            {"name", p.name()},
            {"x", p.state().x},
            {"y", p.state().y},
            {"dx", p.state().dx},
            {"dy", p.state().dy},
            {"ddx", p.state().ddx},
            {"ddy", p.state().ddy},
            {"size", p.state().size},
            {"score", p.score()},
            {"best_score", p.best_score()},
            {"is_me", is_me},
            {"alive", p.alive()},
            {"fake", p.fake()},
        }));
    }

    return state;
}

net::awaitable<void> world_t::on_run()
{
    auto executor = co_await net::this_coro::executor;
    net::steady_timer timer{executor};
    timer.expires_from_now(std::chrono::seconds{0});

    while (true) {
        update(world_t::refresh_dt);
        timer.expires_at(timer.expires_at() + refresh_dt);
        co_await timer.async_wait(net::use_awaitable);
    }
}

void world_t::update(std::chrono::nanoseconds dt)
{
    // update player positions
    for (auto& p : players_) {
        if (!p->alive()) {
            continue;
        }
        if (p->fake()) {
            update_fake_player_dd(*p);
        }
        p->update_pos(dt);
    }

    // check for collisions
    for (auto player_it = begin(players_); player_it != end(players_);
         ++player_it) {
        auto& player = **player_it;
        if (!player.alive()) {
            continue;
        }

        // kill players that are outside the world
        if (!player.is_in_world()) {
            player.kill();
        }

        // compute collisions
        for (auto other_it = ++decltype(player_it)(player_it);
             other_it != end(players_);
             ++other_it) {
            auto& other = **other_it;
            if (!other.alive() || !player.collides(other)) {
                continue;
            }

            if (player.speed() > other.speed()) {
                player.add_score(other.score());
                other.kill();
            }
            else {
                other.add_score(player.score());
                player.kill();
            }
        }
    }

    // respawn killed fake players
    for (const auto& fake_player : fake_players_) {
        if (!fake_player->alive()) {
            fake_player->respawn();
        }
    }
}

void world_t::update_fake_player_dd(player_t& p)
{
    constexpr double emergency_dist = 0.15;
    const auto l1_dist_to = [&p](double x, double y) {
        return std::abs(p.state().x - x) + std::abs(p.state().y - y);
    };

    // initial closest target is the center of the map
    // this way fake players will more likely stay close to the middle
    double closest_x = 0.5;
    double closest_y = 0.5;
    double closest_distance = l1_dist_to(closest_x, closest_y);

    for (const auto& other_ptr : players_) {
        auto& other = *other_ptr;
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
    auto dx = closest_x - p.state().x;
    auto dy = closest_y - p.state().y;

    // override targets for players close to the edge
    if (p.state().x < emergency_dist) {
        dx = 1;
    }
    else if (p.state().x > 1 - emergency_dist) {
        dx = -1;
    }
    if (p.state().y < emergency_dist) {
        dy = 1;
    }
    else if (p.state().y > 1 - emergency_dist) {
        dy = -1;
    }

    p.set_dd(
        fake_player_speed_factor * dx * player_t::max_dd,
        fake_player_speed_factor * dy * player_t::max_dd);
}

} // sd