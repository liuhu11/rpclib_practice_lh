#include <atomic>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <utility>

#include "detail/async_writer.h"
#include "client.h"
#include "config.h"
#include "detail/dev_utils.h"
#include "detail/log.h"
#include "detail/response.h"
#include "rpc_error.h"

using boost::asio::buffer;
using boost::asio::io_context;
using boost::asio::ip::tcp;
using boost::system::error_code;
using msgpack::sbuffer;
using msgpack::unpacked;
using msgpack::unpacker;
using rpc::detail::name_thread;
using resolve_result = boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp>;

namespace rpc{
static constexpr uint32_t default_buffer_size = rpc::Constants::DEFAULT_BUFFER_SIZE;

struct Client::impl {
public:
    using call_t = std::pair<std::string, rpc_promise>;
public:
    // 方便拿到Client 虽然暂时没用
    // 参考Server::impl中利用parent构造ServerSession
    Client* parent;
    io_context io;
    io_context::strand strand;
    // 可能会有多个线程同时call
    std::atomic<int> call_idx;
    std::unordered_map<int, call_t> ongoing_calls;
    std::string addr;
    uint16_t port;
    unpacker unpac;

    // 这个atomic感觉不需要
    bool is_connected;
    std::condition_variable conn_finished;
    // 好像主要是为了防止异步的connect没搞完
    std::mutex mut_conn_finished;

    // 此处有io_thread造成的多线程问题
    std::thread io_thread;
    std::atomic<Client::ConnectionState> state;
    std::shared_ptr<detail::AsyncWriter> writer;
    std::optional<int64_t> timeout_val;
    std::optional<error_code> conn_ec;
    RPC_CREATE_LOG_CHANNEL(Client)
public:
    impl(Client* parent_param, const std::string& addr_param, uint16_t port_param);
    void do_connect(resolve_result endpoints);
    void do_read();
    Client::ConnectionState connection_state() const;
    // 直接调用这个应该没有多线程保护
    void write(sbuffer &&item);
    std::optional<int64_t> timeout() const;
    void timeout(int64_t val);
    void clear_timeout();
};

Client::impl::impl(Client* parent_param, const std::string& addr_param, uint16_t port_param)
    :parent(parent_param), io(), strand(io), call_idx(0), addr(addr_param), port(port_param),
    is_connected(false), state(Client::ConnectionState::initial), 
    // 此时这个socket只是被构造，还没有打开
    writer(std::make_shared<detail::AsyncWriter>(&io, tcp::socket(io))) {
        unpac.reserve_buffer(default_buffer_size);
}

void Client::impl::do_connect(resolve_result endpoints) {
    LOG_INFO("Initialting connection.")
    conn_ec = std::nullopt;

    // 异步执行完毕后 socket打开
    boost::asio::async_connect(writer->socket(), endpoints, 
        [this](error_code ec, tcp::socket::endpoint_type){
            if(!ec) {
                std::unique_lock<std::mutex> lock(mut_conn_finished);
                LOG_INFO("Client connected to {}:{}", addr, port);
                is_connected = true;
                state.store(Client::ConnectionState::connected);
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
    // 原作者认为这里没必要捕获 因为max_read_bytes是constexpr
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
                    if(response.error().get() != nullptr) {
                        throw RpcError("rpc::RpcError during call", current_call.first, 
                            response.error());
                    }
                    current_call.second.set_value(std::move(*response.result()));
                }
                catch(...) {
                    current_call.second.set_exception(std::current_exception());
                }
                strand.post([this, id](){
                    ongoing_calls.erase(id);
                });
            }
            // resizing strategy: if the remaining buffer size is
            // less than the maximum bytes requested from asio,
            // then request max_read_bytes. This prompts the unpacker
            // to resize its buffer doubling its size
            if(unpac.buffer_capacity() < max_read_bytes) {
                LOG_TRACE("Reserving extra buffer: {}", max_read_bytes)
                unpac.reserve_buffer(max_read_bytes);
            }
            do_read();
        }
        else if(ec == boost::asio::error::eof) {
            LOG_WARN("The server closed the connection.")
            state = Client::ConnectionState::disconnected;
        }
        else if(ec == boost::asio::error::connection_reset) {
            // ?
            // Yes, this should be connection_state::reset,
            // but on windows, disconnection results in reset. May be
            // asio bug, may be a windows socket pecularity. Should be
            // investigated later.
            LOG_WARN("The server was reset or disconencted");
            state = Client::ConnectionState::disconnected;
        }
        else {
            LOG_ERROR("Unhandled error code: {} | {}", ec, ec.message());
        }
    });
}

Client::ConnectionState Client::impl::connection_state() const {
    return state.load();
}

void Client::impl::write(sbuffer &&item) {
    writer->write(std::move(item));
}

std::optional<int64_t> Client::impl::timeout() const {
    return timeout_val;
}

void Client::impl::timeout(int64_t val) {
    timeout_val = val;
}

void Client::impl::clear_timeout() {
    timeout_val.reset();
}
}

namespace rpc {
void Client::wait_conn() {
    std::unique_lock<std::mutex> lock(pimpl_->mut_conn_finished);
    // 防止假唤醒
    while(pimpl_->is_connected == false) {
        // 还要判断这个 用while挺好的
        // 否则可以使用带 Predicate 的重载
        auto ec = pimpl_->conn_ec;
        if(ec.has_value()) {
            throw rpc::SystemError(ec.value());
        }

        auto timeout_val = pimpl_->timeout_val;
        if(timeout_val.has_value()) {
            auto wait_ret = pimpl_->conn_finished.wait_for(lock, 
                std::chrono::milliseconds(timeout_val.value()));
            if(wait_ret == std::cv_status::timeout) {
                throw rpc::Timeout(std::format("Timeout of {}ms while connecting to {} | {}", 
                    timeout_val.value(),pimpl_->addr, pimpl_->port));
            }
        }
        else {
            pimpl_->conn_finished.wait(lock);
        }
    }
}

void Client::post(std::shared_ptr<msgpack::sbuffer> buffer) {
    // 隐式捕获了this
    // 保证有序的申请写
    pimpl_->strand.post([=](){
        pimpl_->write(std::move(*buffer));
    });
}

// 真正的rpc通信还是只发送了sbuffer -- 序列化后的数据
void Client::post(std::shared_ptr<msgpack::sbuffer> buffer, int idx, 
    const std::string& func_name, std::shared_ptr<rpc_promise> p) {
        pimpl_->strand.post([=](){
            pimpl_->ongoing_calls[idx] = std::make_pair(func_name, std::move(*p));
            pimpl_->write(std::move(*buffer));
        });
}

Client::Client(const std::string& addr, uint16_t port)
    :pimpl_(std::make_unique<impl>(this, addr, port)) {
        tcp::resolver resolver(pimpl_->io);
        // 返回的是个range 但也可以当iter用
        auto endpoints = resolver.resolve(pimpl_->addr, std::to_string(pimpl_->port));
        pimpl_->do_connect(endpoints);
        std::thread io_thread([this](){
            RPC_CREATE_LOG_CHANNEL(Client)
            name_thread("client");
            pimpl_->io.run();
        });
        pimpl_->io_thread = std::move(io_thread);
}

Client::~Client() {
    pimpl_->io.stop();
    if(pimpl_->io_thread.joinable()) {pimpl_->io_thread.join();}
}

std::optional<int64_t> Client::timeout() const {
    return pimpl_->timeout();
}

void Client::timeout(int64_t val) {
    pimpl_->timeout(val);
}

void Client::clear_timeout() {
    pimpl_->clear_timeout();
}

Client::ConnectionState Client::connection_state() const {
    return pimpl_->connection_state();
}

// void Client::wait_all_responses() {
//     // 如何防止对同一个promise get_future两次的
//     // async_call有一次
//     // 感觉好像没意义
//     for(auto& call : pimpl_->ongoing_calls) {
//         call.second.second.get_future().wait();
//     }
// }

int Client::next_call_idx() {
    return ++(pimpl_->call_idx);
}

[[noreturn]] void Client::throw_timeout(const std::string& func_name) {
    throw Timeout(std::format("Timeout of {}ms while calling rpc function '{}'", 
        timeout().value(), func_name));
}

}