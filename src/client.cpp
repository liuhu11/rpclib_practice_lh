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
#include "response.h"

using boost::asio::buffer;
using boost::asio::io_context;
using boost::asio::ip::tcp;
using boost::system::error_code;
using msgpack::sbuffer;
using msgpack::unpacked;
using msgpack::unpacker;

namespace rpc{
static constexpr uint32_t default_buffer_size = rpc::Constants::DEFAULT_BUFFER_SIZE;

struct Client::impl {
public:
    using call_t = std::pair<std::string, rpc_promise>;
public:
    // 方便调用Client类的接口
    // 避免循环引用
    Client* parent;
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
public:
    impl(Client* parent_param, const std::string& addr_param, uint16_t port_param);
    void do_connect(tcp::resolver::iterator endpoint_iterator);
    void do_read();
    Client::ConnectionState connection_state() const;
    void write(sbuffer &&item);
    std::optional<int64_t> timeout() const;
    void timeout(int64_t val);
    void clear_timeout();
};

Client::impl::impl(Client* parent_param, const std::string& addr_param, uint16_t port_param)
    :parent(parent_param), io(), strand(io), call_idx(0), addr(addr_param), port(port_param),
    is_connected(false), state(Client::ConnectionState::initial), 
    writer(std::make_shared<detail::AsyncWriter>(&io, tcp::socket(io))) {
        unpac.reserve_buffer(default_buffer_size);
}

void Client::impl::do_connect(tcp::resolver::iterator endpoint_iterator) {
    LOG_INFO("Initialting connection.")
    conn_ec = std::nullopt;

    boost::asio::async_connect(writer->socket(), endpoint_iterator, 
        [this](error_code ec, tcp::resolver::iterator){
            if(!ec) {
                std::unique_lock<std::mutex> lock(mut_conn_finished);
                LOG_INFO("Client connected to {}:{}", addr, port);
                is_connected.store(true, std::memory_order_release);
                state.store(Client::ConnectionState::connected, std::memory_order_release);
                conn_finished.notify_all();
                do_read();
            }
            else {
                std::unique_lock<std::mutex> lock(mut_conn_finished);
                LOG_ERROR("Error during connection: {}", ec);
                state.store(Client::ConnectionState::disconnected);
                conn_ec = ec;
                conn_finished.notify_all();
            }
        });
}

void Client::impl::do_read() {
    LOG_TRACE("do_read")
    constexpr size_t max_read_bytes = default_buffer_size;
    writer->socket().async_read_some(buffer(unpac.buffer(), max_read_bytes),
    [this, max_read_bytes](error_code ec, size_t lenth){
        if(!ec) {
            LOG_TRACE("Read chunk of size {}", length)

            // 含义：消耗了多少缓冲区
            unpac.buffer_consumed(lenth);
            // unpacked就是object_handle
            unpacked result;
            // 如果成功解码出一个对象，next() 会将其存储到 result 参数中，并返回 true
            while(unpac.next(result)) {
                auto response = detail::Response(std::move(result));
                auto id = response.id();
                auto& current_call = ongoing_calls[id];
                try {
                    // todo 看看能不能抛出更明确的报错
                    if(response.error() != nullptr) {
                        
                    }
                }
            }
        }
    });
}


}