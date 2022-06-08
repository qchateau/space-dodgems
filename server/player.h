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
        static constexpr double size{0.01};

        double x, y, dx, dy, ddx, ddy;
    };
    static constexpr double max_dd = 5;

    player_t(world_t& world, id_t id, std::string_view name, bool fake);
    ~player_t() = default;

    player_t(const player_t&) = delete;
    player_t(player_t&&) = delete;

    player_t& operator=(const player_t&) = delete;
    player_t& operator=(player_t&&) = delete;

    bool operator==(const player_t&) const;
    bool operator!=(const player_t&) const;

    void set_pos(double x, double y);
    void set_dd(double ddx, double ddy);
    void respawn();
    void add_score(double v);
    void update_pos(std::chrono::nanoseconds dt);

    [[nodiscard]] const auto& state() const { return state_; };
    [[nodiscard]] id_t id() const { return id_; }
    [[nodiscard]] const std::string& name() const { return name_; }
    [[nodiscard]] bool alive() const { return alive_; }
    [[nodiscard]] bool fake() const { return fake_; }
    [[nodiscard]] double score() const { return score_; }
    [[nodiscard]] double best_score() const { return best_score_; }
    [[nodiscard]] double speed() const;
    [[nodiscard]] double distance_to(const player_t& other) const;

    [[nodiscard]] bool is_in_world() const;
    [[nodiscard]] bool collides(const player_t& other) const;
    void kill();

private:
    world_t& world_;

    std::mt19937 rnd_gen_{std::random_device{}()};
    const id_t id_;
    const std::string name_;
    double score_;
    double best_score_;
    bool fake_;
    state_t state_;
    double acc_;
    bool alive_;
};

} // sd
