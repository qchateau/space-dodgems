#pragma once

#include <future>
#include <list>
#include <memory>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "config.h"
#include "player.h"

namespace si {

class world : public std::enable_shared_from_this<world> {
public:
    typename player::handle register_player();

    world(net::io_context& ioc);
    void run();

    nlohmann::json game_state_for_player(const player::handle& player);

private:
    void unregister_player(const player& p);

    net::awaitable<void> on_run();
    void update(std::chrono::nanoseconds dt);
    bool is_in_world(const player& p) const;

    net::io_context& ioc_;
    std::list<player> players_;
};

} // si
