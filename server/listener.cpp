#include "listener.h"
#include "session.h"
#include "world.h"

namespace si {

listener_t::listener_t(net::io_context& ioc, world_t& world, tcp::endpoint endpoint)
    : ioc_{ioc}, acceptor_{ioc}, world_{world}
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
        "listening on {}:{}",
        acceptor_.local_endpoint().address().to_string(),
        acceptor_.local_endpoint().port());

    while (true) {
        auto socket = co_await acceptor_.async_accept(net::use_awaitable);
        std::make_shared<session_t>(world_, std::move(socket))->run();
    }
}

} // si