#pragma once

#include <future>
#include <list>
#include <memory>
#include <random>
#include <vector>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "config.h"

namespace si {

class world_t : public std::enable_shared_from_this<world_t> {
public:
    static constexpr std::size_t max_players = 8;

    world_t(net::io_context& ioc);
    ~world_t();
    void run();

    nlohmann::json game_state_for_player(const player_handle_t& player);
    player_handle_t register_player();
    int real_players() const;
    int available_places() const;

private:
    player_handle_t register_player(bool fake);
    void unregister_player(const player_t& p);
    void adjust_players();

    net::awaitable<void> on_run();
    void update(std::chrono::nanoseconds dt);
    void update_fake_player_dd(player_t& p);

    net::io_context& ioc_;
    std::vector<std::unique_ptr<player_t>> players_;
    std::list<player_handle_t> fake_players_;
};

} // si
