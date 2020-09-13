#pragma once

#include <future>
#include <list>
#include <memory>
#include <spdlog/spdlog.h>

#include "config.h"
#include "player.h"

namespace si {

class world : public std::enable_shared_from_this<world> {
public:
    typename player::handle registerPlayer();

    world(net::io_context& ioc);
    const auto& players() const { return players_; }
    void run();

private:
    net::awaitable<void> on_run();
    void update(std::chrono::nanoseconds dt);

    net::io_context& ioc_;
    std::list<player> players_;
};

} // si
