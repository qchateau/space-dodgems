#include "listener.h"
#include "session.h"

namespace si {

listener::listener(net::io_context& ioc, tcp::endpoint endpoint)
    : ioc_(ioc), acceptor_(ioc)
{
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(net::socket_base::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen(net::socket_base::max_listen_connections);
}

void listener::run()
{
    net::co_spawn(
        ioc_,
        [self = shared_from_this()]() -> net::awaitable<void> {
            co_await self->on_run();
        },
        net::detached);
}

net::awaitable<void> listener::on_run()
{
    spdlog::info(
        "listening on {}:{}",
        acceptor_.local_endpoint().address().to_string(),
        acceptor_.local_endpoint().port());

    while (true) {
        // The new connection gets its own strand
        auto socket = co_await acceptor_.async_accept(
            net::make_strand(ioc_), net::use_awaitable);

        // Create the http session and run it
        std::make_shared<session>(std::move(socket))->run();
    }
}

} // si