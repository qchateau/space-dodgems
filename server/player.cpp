#include "player.h"
#include "world.h"

#include <algorithm>

namespace sd {

player_t::player_t(world_t& world, id_t id, std::string_view name, bool fake)
    : id_{id},
      name_{name},
      fake_{fake},
      state_{.x = 0.5, .y = 0.5, .dx = 0, .dy = 0, .ddx = 0, .ddy = 0},
      acc_{0.02},
      world_{world}
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

void player_t::set_pos(double x, double y)
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
    std::uniform_real_distribution<> rnd(0.1, 0.9);

    state_.dx = state_.dy = state_.ddx = state_.ddy = 0;
    state_.x = rnd(rnd_gen_);
    state_.y = rnd(rnd_gen_);
    alive_ = true;
    score_ = 0;
    birth_time_ = clock_t::now();
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
    const auto size = state_.size / 2 + other.state_.size / 2;
    const auto dist = std::sqrt(std::norm(
        std::complex{other.state_.x - state_.x, other.state_.y - state_.y}));
    return dist < size;
}

void player_t::kill()
{
    alive_ = false;
}

} // sd