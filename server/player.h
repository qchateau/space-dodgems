#pragma once

#include <atomic>
#include <list>
#include <memory>
#include <spdlog/spdlog.h>

#include "config.h"

namespace si {

class world_t;

class player_t {
public:
    using id_t = uint64_t;
    using clock_t = std::chrono::steady_clock;
    using handle_t = std::unique_ptr<player_t, std::function<void(player_t*)>>;
    struct state_t {
        const double width{0.01};
        const double height{0.01};

        double x, y, dx, dy, ddx, ddy;

        double l1_speed() const;
    };

    player_t(world_t& world);

    player_t(const player_t&) = delete;
    player_t(player_t&&) = delete;

    player_t& operator=(const player_t&) = delete;
    player_t& operator=(player_t&&) = delete;

    bool operator==(const player_t&) const;
    bool operator!=(const player_t&) const;

    void set_pos(double x, double y);
    void set_dd(double ddx, double ddy);

    void update_pos(std::chrono::nanoseconds dt);
    const auto& state() const { return state_; };
    id_t id() const { return id_; }
    bool alive() const { return alive_; }
    std::chrono::nanoseconds lifetime() const;

    bool is_in_world() const;
    bool collides(const player_t& other) const;
    void kill();

private:
    static std::atomic<id_t> id_generator_;

    world_t& world_;

    id_t id_;
    clock_t::time_point birth_time_;
    state_t state_;
    double acc_;
    bool alive_;
};
} // si
