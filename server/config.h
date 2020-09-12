#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>

namespace si {

namespace beast = boost::beast;
namespace net = boost::asio;
namespace http = beast::http;
namespace websocket = beast::websocket;
using tcp = net::ip::tcp;

template <typename T>
struct use_awaitable_executor {
    using type = typename net::use_awaitable_t<T>::template as_default_on_t<T>;
};

template <typename T>
using use_awaitable_executor_t = typename use_awaitable_executor<T>::type;

} // si
