#include "listener.h"
#include "session.h"
#include "world.h"

namespace de {

listener_t::listener_t(
    net::io_context& ioc,
    std::vector<std::shared_ptr<world_t>> worlds,
    tcp::endpoint endpoint)
    : ioc_{ioc}, acceptor_{ioc}, worlds_{std::move(worlds)}
{
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(net::socket_base::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen(net::socket_base::max_listen_connections);
}

void listener_t::run()
{
    net::co_spawn(
        ioc_,
        [self = shared_from_this()]() -> net::awaitable<void> {
            co_await self->on_run();
        },
        net::detached);
}

net::awaitable<void> listener_t::on_run()
{
    spdlog::info(
        "listening on {}:{}, generated {} worlds",
        acceptor_.local_endpoint().address().to_string(),
        acceptor_.local_endpoint().port(),
        worlds_.size());

    while (true) {
        auto socket = co_await acceptor_.async_accept(net::use_awaitable);
        bool found_world = false;
        for (auto& world_ptr : worlds_) {
            if (world_ptr->available_places() == 0) {
                continue;
            }

            std::make_shared<session_t>(world_ptr, std::move(socket))->run();
            found_world = true;
            break;
        }
    }
}

} // de