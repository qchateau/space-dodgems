#pragma once

#include <list>
#include <memory>
#include <random>
#include <string>
#include <string_view>
#include <spdlog/spdlog.h>

#include "config.h"

namespace sd {

class player_t {
public:
    using id_t = player_id_t;
    using clock_t = std::chrono::steady_clock;
    struct state_t {
        const double size{0.01};

        double x, y, dx, dy, ddx, ddy;
    };
    static constexpr double max_dd = 5;

    player_t(world_t& world, id_t id, std::string_view name, bool fake);

    player_t(const player_t&) = delete;
    player_t(player_t&&) = delete;

    player_t& operator=(const player_t&) = delete;
    player_t& operator=(player_t&&) = delete;

    bool operator==(const player_t&) const;
    bool operator!=(const player_t&) const;

    void set_pos(double x, double y);
    void set_dd(double ddx, double ddy);
    void add_score(double v) { score_ += v; }
    void respawn();
    void update_pos(std::chrono::nanoseconds dt);

    const auto& state() const { return state_; };
    id_t id() const { return id_; }
    std::string name() const { return name_; }
    bool alive() const { return alive_; }
    bool fake() const { return fake_; }
    std::chrono::nanoseconds lifetime() const;
    double score() const { return score_; }
    double speed() const;
    double distance_to(const player_t& other) const;

    bool is_in_world() const;
    bool collides(const player_t& other) const;
    void kill();

private:
    world_t& world_;

    std::mt19937 rnd_gen_{std::random_device{}()};
    const id_t id_;
    const std::string name_;
    clock_t::time_point birth_time_;
    double score_;
    bool fake_;
    state_t state_;
    double acc_;
    bool alive_;
};

} // sd
