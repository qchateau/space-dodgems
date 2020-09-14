#include "world.h"

namespace si {

std::atomic<player::id_t> player::id_generator_{0};

player::player(world& world)
    : id_{id_generator_.fetch_add(1)},
      state_{.x = 0.5, .y = 0.5, .dx = 0, .dy = 0, .ddx = 0, .ddy = 0},
      speed_{0.25},
      acc_{0.25},
      alive_{true},
      world_{world}
{
}

bool player::operator==(const player& other) const
{
    return id_ == other.id_;
}

bool player::operator!=(const player& other) const
{
    return id_ != other.id_;
}

void player::update_pos(std::chrono::nanoseconds dt)
{
    double seconds =
        std::chrono::duration_cast<std::chrono::duration<double>>(dt).count();
    state_.dx += state_.ddx * acc_ * seconds;
    state_.dy += state_.ddy * acc_ * seconds;
    state_.x += state_.dx * speed_ * seconds;
    state_.y += state_.dy * speed_ * seconds;
}

void player::to_left()
{
    state_.ddx -= 1;
}

void player::to_right()
{
    state_.ddx += 1;
}

void player::to_top()
{
    state_.ddy += 1;
}

void player::to_bottom()
{
    state_.ddy -= 1;
}

void player::kill()
{
    alive_ = false;
}

} // si