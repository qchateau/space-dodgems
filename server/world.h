#pragma once

#include <future>
#include <list>
#include <memory>
#include <random>
#include <vector>

#include <boost/uuid/random_generator.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "config.h"

namespace de {

class player_already_registered : public std::runtime_error {
public:
    player_already_registered() : runtime_error{"player already registered"} {}
};

class world_t : public std::enable_shared_from_this<world_t> {
public:
    static constexpr auto refresh_dt = std::chrono::milliseconds{20};
    static constexpr std::size_t max_players = 8;

    world_t(net::io_context& ioc);
    ~world_t();
    void run();

    nlohmann::json game_state_for_player(const player_handle_t& player);
    player_handle_t register_player(const player_id_t& player_id);
    int real_players() const;
    int available_places() const;

private:
    player_handle_t register_player(const player_id_t& player_id, bool fake);
    void unregister_player(const player_t& p);
    void adjust_players();

    net::awaitable<void> on_run();
    void update(std::chrono::nanoseconds dt);
    void update_fake_player_dd(player_t& p);

    net::io_context& ioc_;
    std::vector<std::unique_ptr<player_t>> players_;
    std::list<player_handle_t> fake_players_;
    boost::uuids::random_generator uuid_generator_;
};

} // de
