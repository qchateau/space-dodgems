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

class world : public std::enable_shared_from_this<world> {
public:
    typename player::handle register_player();

    world(net::io_context& ioc);
    void run();

    nlohmann::json game_state_for_player(const player::handle& player);

private:
    void set_initial_player_pos(player& p);
    void unregister_player(const player& p);

    net::awaitable<void> on_run();
    void update(std::chrono::nanoseconds dt);

    std::mt19937 rnd_gen_{std::random_device{}()};
    net::io_context& ioc_;
    std::list<player> players_;
};

} // si
