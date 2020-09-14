#pragma once

#include <future>
#include <list>
#include <memory>
#include <random>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "config.h"
#include "player.h"

namespace si {

class world_t : public std::enable_shared_from_this<world_t> {
public:
    typename player_t::handle_t register_player();

    world_t(net::io_context& ioc);
    void run();

    nlohmann::json game_state_for_player(const player_t::handle_t& player);

private:
    void set_initial_player_pos(player_t& p);
    void unregister_player(const player_t& p);

    net::awaitable<void> on_run();
    void update(std::chrono::nanoseconds dt);

    std::mt19937 rnd_gen_{std::random_device{}()};
    net::io_context& ioc_;
    std::list<player_t> players_;
};

} // si
