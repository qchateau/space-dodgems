#include "world.h"

#include <algorithm>

namespace si {

std::atomic<player_t::id_t> player_t::id_generator_{0};

player_t::player_t(world_t& world, bool fake)
    : id_{id_generator_.fetch_add(1)},
      birth_time_{clock_t::now()},
      score_{0},
      fake_{fake},
      state_{.x = 0.5, .y = 0.5, .dx = 0, .dy = 0, .ddx = 0, .ddy = 0},
      acc_{0.02},
      alive_{true},
      world_{world}
{
}

bool player_t::operator==(const player_t& other) const
{
    return id_ == other.id_;
}

bool player_t::operator!=(const player_t& other) const
{
    return id_ != other.id_;
}

void player_t::set_pos(double x, double y)
{
    state_.x = x;
    state_.y = y;
}

void player_t::set_dd(double ddx, double ddy)
{
    state_.ddx = std::clamp(ddx, -max_dd, max_dd);
    state_.ddy = std::clamp(ddy, -max_dd, max_dd);
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
    score_ += (std::abs(xinc) + std::abs(yinc)) * 1000;
}

std::chrono::nanoseconds player_t::lifetime() const
{
    return clock_t::now() - birth_time_;
}

double player_t::l1_speed() const
{
    return std::abs(state_.dx) + std::abs(state_.dy);
}

double player_t::l1_distance_to(const player_t& other) const
{
    return std::abs(state_.x - other.state_.x)
           + std::abs(state_.dy - other.state_.y);
}

bool player_t::is_in_world() const
{
    return 0 <= state_.x && state_.x < 1 && 0 <= state_.y && state_.y < 1;
}

bool player_t::collides(const player_t& other) const
{
    const auto width = state_.width / 2 + other.state_.width / 2;
    const auto height = state_.height / 2 + other.state_.height / 2;
    const auto dist_x = std::abs(other.state_.x - state_.x);
    const auto dist_y = std::abs(other.state_.y - state_.y);
    return dist_x < width && dist_y < height;
}

void player_t::kill()
{
    alive_ = false;
}

} // si