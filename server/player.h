#pragma once

#include <atomic>
#include <list>
#include <memory>
#include <spdlog/spdlog.h>

#include "config.h"

namespace si {

class world;

class player {
public:
    using id_t = uint64_t;
    using handle = std::unique_ptr<player, std::function<void(player*)>>;
    struct state_t {
        const double width{0.01};
        const double height{0.01};

        double x, y, dx, dy, ddx, ddy;

        double l1_speed() const;
    };
    using clock_t = std::chrono::steady_clock;

    player(world& world);

    player(const player&) = delete;
    player(player&&) = delete;

    player& operator=(const player&) = delete;
    player& operator=(player&&) = delete;

    bool operator==(const player&) const;
    bool operator!=(const player&) const;

    void set_pos(double x, double y);
    void update_pos(std::chrono::nanoseconds dt);
    const auto& state() const { return state_; };
    id_t id() const { return id_; }
    bool alive() const { return alive_; }
    std::chrono::nanoseconds lifetime() const;

    bool is_in_world() const;
    bool collides(const player& other) const;

    void to_left();
    void to_right();
    void to_top();
    void to_bottom();
    void kill();

private:
    static std::atomic<id_t> id_generator_;

    world& world_;

    id_t id_;
    clock_t::time_point birth_time_;
    state_t state_;
    double acc_;
    bool alive_;
};
} // si
