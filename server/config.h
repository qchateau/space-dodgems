#pragma once

#include <functional>
#include <memory>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/uuid/uuid.hpp>

namespace sd {

namespace beast = boost::beast; // NOLINT
namespace net = boost::asio; // NOLINT
namespace http = beast::http; // NOLINT
namespace websocket = beast::websocket; // NOLINT
using tcp = net::ip::tcp;

template <typename T>
struct use_awaitable_executor {
    using type = typename net::use_awaitable_t<T>::template as_default_on_t<T>;
};

template <typename T>
using use_awaitable_executor_t = typename use_awaitable_executor<T>::type;

class session_t;
class listener_t;
class world_t;
class player_t;

using player_id_t = boost::uuids::uuid;
using player_handle_t = std::unique_ptr<player_t, std::function<void(player_t*)>>;

} // sd
