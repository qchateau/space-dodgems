#include "world.h"

namespace si {

std::atomic<player::id_t> player::id_generator_{0};

player::player(world& world)
    : id_{id_generator_.fetch_add(1)}, x_{0}, y_{0}, dx_{0}, dy_{0}, speed_{100}, world_{world}
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
    x_ += dx_ * speed_ * seconds;
    y_ += dy_ * speed_ * seconds;
}

void player::to_left()
{
    dx_ = -1;
    dy_ = 0;
}

void player::to_right()
{
    dx_ = 1;
    dy_ = 0;
}

void player::to_top()
{
    dx_ = 0;
    dy_ = 1;
}

void player::to_bottom()
{
    dx_ = 0;
    dy_ = -1;
}

} // si