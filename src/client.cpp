#include <atomic>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <utility>

#include "async_writer.h"
#include "client.h"
#include "config.h"
#include "log.h"

using boost::asio::io_context;
using boost::system::error_code;
using msgpack::unpacker;

namespace rpc{
static constexpr uint32_t default_buffer_size = rpc::Constants::DEFAULT_BUFFER_SIZE;

struct Client::impl {
public:
    using call_t = std::pair<std::string, rpc_promise>;
public:
    io_context io;
    io_context::strand strand;
    std::atomic<int> call_idx;
    std::unordered_map<int, call_t> ongoing_calls;
    std::string addr;
    uint16_t port;
    unpacker unpac;
    std::atomic<bool> is_connected;
    std::condition_variable conn_finished;
    std::mutex mut_conn_finished;
    std::thread io_thread;
    std::atomic<Client::ConnectionState> state;
    std::shared_ptr<detail::AsyncWriter> writer;
    std::optional<int64_t> timeout;
    std::optional<error_code> conn_ec;
    RPC_CREATE_LOG_CHANNEL(Client)

};
}