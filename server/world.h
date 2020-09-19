#pragma once

#include <future>
#include <list>
#include <memory>
#include <random>
#include <string_view>
#include <vector>

#include <boost/uuid/random_generator.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "config.h"

namespace sd {

class player_already_registered : public std::runtime_error {
public:
    player_already_registered() : runtime_error{"player already registered"} {}
};

class world_t : public std::enable_shared_from_this<world_t> {
public:
    static constexpr auto refresh_dt = std::chrono::milliseconds{20};

    world_t(net::io_context& ioc);
    ~world_t();
    void run();

    nlohmann::json game_state_for_player(const player_handle_t& player);
    player_handle_t register_player(
        const player_id_t& player_id,
        std::string_view player_name);
    int real_players() const;
    int active_real_players() const;
    int available_places() const;

private:
    using clock_t = std::chrono::steady_clock;
    struct idle_player {
        clock_t::time_point from;
        std::unique_ptr<player_t> player;
    };

    player_handle_t register_player(
        const player_id_t& player_id,
        std::string_view player_name,
        bool fake);
    void unregister_player(const player_t& p);
    void adjust_players();

    net::awaitable<void> update_loop();
    net::awaitable<void> check_idle_players_loop();

    void update(std::chrono::nanoseconds dt);
    void update_fake_player_dd(player_t& p);
    void check_idle_players();

    net::io_context& ioc_;
    std::vector<std::unique_ptr<player_t>> players_;
    std::vector<idle_player> idle_players_;
    std::list<player_handle_t> fake_players_;
    boost::uuids::random_generator uuid_generator_;
};

} // sd
