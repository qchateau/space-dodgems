#include "player.h"
#include "world.h"

#include <algorithm>

namespace sd {
namespace {
constexpr auto score_multiplier = 1000;
}

player_t::player_t(world_t& world, id_t id, std::string_view name, bool fake)
    : id_{id},
      name_{name},
      score_{0},
      best_score_{0},
      fake_{fake},
      state_{.x = 0.5, .y = 0.5, .dx = 0, .dy = 0, .ddx = 0, .ddy = 0}, // NOLINT(*-magic-numbers)
      acc_{0.02}, // NOLINT(*-magic-numbers)
      world_{world},
      alive_{false}
{
    respawn();
}

bool player_t::operator==(const player_t& other) const
{
    return id_ == other.id_;
}

bool player_t::operator!=(const player_t& other) const
{
    return id_ != other.id_;
}

void player_t::set_pos(double x, double y) // NOLINT(bugprone-*)
{
    state_.x = x;
    state_.y = y;
}

void player_t::set_dd(double ddx, double ddy)
{
    const double norm = std::sqrt(std::norm(std::complex<double>(ddx, ddy)));
    if (norm > max_dd) {
        ddx = max_dd * (ddx / norm);
        ddy = max_dd * (ddy / norm);
    }
    state_.ddx = ddx;
    state_.ddy = ddy;
}

void player_t::respawn()
{
    constexpr auto low_bound = 0.1;
    constexpr auto high_bound = 0.9;
    std::uniform_real_distribution<> rnd(low_bound, high_bound);

    state_.dx = state_.dy = state_.ddx = state_.ddy = 0;
    state_.x = rnd(rnd_gen_);
    state_.y = rnd(rnd_gen_);
    alive_ = true;
    score_ = 0;
}

void player_t::add_score(double v)
{
    score_ += v;
    best_score_ = std::max(best_score_, score_);
}

void player_t::update_pos(std::chrono::nanoseconds dt)
{
    double seconds =
        std::chrono::duration_cast<std::chrono::duration<double>>(dt).count();
    state_.dx += state_.ddx * acc_ * seconds;
    state_.dy += state_.ddy * acc_ * seconds;
    const auto xinc = state_.dx * seconds;
    const auto yinc = state_.dy * seconds;
    state_.x += xinc;
    state_.y += yinc;
    add_score((std::abs(xinc) + std::abs(yinc)) * score_multiplier);
}

double player_t::speed() const
{
    return std::sqrt(std::norm(std::complex{state_.dx, state_.dy}));
}

double player_t::distance_to(const player_t& other) const
{
    return std::sqrt(std::norm(std::complex{
        state_.x - other.state_.x,
        state_.dy - other.state_.y,
    }));
}

bool player_t::is_in_world() const
{
    return 0 <= state_.x && state_.x < 1 && 0 <= state_.y && state_.y < 1;
}

bool player_t::collides(const player_t& other) const
{
    const auto size = player_t::state_t::size;
    const auto dist = std::sqrt(std::norm(
        std::complex{other.state_.x - state_.x, other.state_.y - state_.y}));
    return dist < size;
}

void player_t::kill()
{
    alive_ = false;
}

} // sd
