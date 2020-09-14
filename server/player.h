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
        double x, y, dx, dy, ddx, ddy;
    };

    player(world& world);

    player(const player&) = delete;
    player(player&&) = delete;

    player& operator=(const player&) = delete;
    player& operator=(player&&) = delete;

    bool operator==(const player&) const;
    bool operator!=(const player&) const;

    void update_pos(std::chrono::nanoseconds dt);
    const auto& state() const { return state_; };
    id_t id() const { return id_; }
    bool alive() const { return alive_; }

    void to_left();
    void to_right();
    void to_top();
    void to_bottom();
    void kill();

private:
    static std::atomic<id_t> id_generator_;

    world& world_;

    id_t id_;
    state_t state_;
    double speed_;
    double acc_;
    bool alive_;
};
} // si
