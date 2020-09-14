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

    player(world& world);

    player(const player&) = delete;
    player(player&&) = delete;

    player& operator=(const player&) = delete;
    player& operator=(player&&) = delete;

    bool operator==(const player&) const;
    bool operator!=(const player&) const;

    void update_pos(std::chrono::nanoseconds dt);
    auto x() const { return x_; }
    auto y() const { return y_; }
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
    double x_;
    double y_;
    double dx_;
    double dy_;
    double speed_;
    bool alive_;
};
} // si
