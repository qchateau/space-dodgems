#include "world.h"

namespace si {

std::atomic<player_t::id_t> player_t::id_generator_{0};

double player_t::state_t::l1_speed() const
{
    return std::abs(dx) + std::abs(dy);
}

player_t::player_t(world_t& world)
    : id_{id_generator_.fetch_add(1)},
      birth_time_{clock_t::now()},
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

void player_t::update_pos(std::chrono::nanoseconds dt)
{
    double seconds =
        std::chrono::duration_cast<std::chrono::duration<double>>(dt).count();
    state_.dx += state_.ddx * acc_ * seconds;
    state_.dy += state_.ddy * acc_ * seconds;
    state_.x += state_.dx * seconds;
    state_.y += state_.dy * seconds;
}

std::chrono::nanoseconds player_t::lifetime() const
{
    return clock_t::now() - birth_time_;
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

void player_t::to_left()
{
    state_.ddx = std::min(.0, state_.ddx - 1);
}

void player_t::to_right()
{
    state_.ddx = std::max(.0, state_.ddx + 1);
}

void player_t::to_top()
{
    state_.ddy = std::max(.0, state_.ddy + 1);
}

void player_t::to_bottom()
{
    state_.ddy = std::min(.0, state_.ddy - 1);
}

void player_t::kill()
{
    alive_ = false;
}

} // si