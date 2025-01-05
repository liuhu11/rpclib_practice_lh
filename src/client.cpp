#include <atomic>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <utility>
#include <functional>

#include "detail/async_writer.h"
#include "client.h"
#include "config.h"
#include "detail/dev_utils.h"
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
using endpoints_range = boost::asio::ip::tcp::resolver::results_type;

namespace rpc{
logging::DefaultLogger Client::logger_{logging::LoggerFactory<>::create_logger("Client")};
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
    // std::condition_variable conn_finished;
    // 好像主要是为了防止异步的connect没搞完
    // std::mutex mut_conn_finished;

    // 此处有io_thread造成的多线程问题
    std::thread io_thread;
    std::atomic<Client::ConnectionState> state;
    std::shared_ptr<detail::AsyncWriter> writer;
    std::function<bool()> reconnectable;
    std::optional<int64_t> timeout_val;
    // std::optional<error_code> conn_ec;
    std::vector<msgpack::sbuffer> pending_calls;
public:
    impl(Client* parent_param, const std::string& addr_param, uint16_t port_param, std::function<bool()>&& reconnectable);
    void do_connect(endpoints_range endpoints);
    void do_connect(endpoints_range endpoints, std::function<bool()>&& reconnectable);
    void do_read();
    Client::ConnectionState connection_state() const;
    // 直接调用这个应该没有多线程保护
    void write(sbuffer &&item);
    std::optional<int64_t> timeout() const;
    void timeout(int64_t val);
    void clear_timeout();
};

Client::impl::impl(Client* parent_param, const std::string& addr_param, uint16_t port_param, std::function<bool()>&& reconnectable)
    :parent(parent_param), io(), strand(io), call_idx(0), addr(addr_param), port(port_param),
    is_connected(false), state(Client::ConnectionState::initial), 
    // 此时这个socket只是被构造，还没有打开
    // mutable允许lambda修改复制捕获的对象，以及调用它们的非 const 成员函数。
    writer(std::make_shared<detail::AsyncWriter>(&io, tcp::socket(io))), reconnectable(std::move(reconnectable)) {
        unpac.reserve_buffer(default_buffer_size);
}

void Client::impl::do_connect(endpoints_range endpoints, std::function<bool()>&& reconnectable) {
    boost::asio::async_connect(writer->socket(), endpoints, 
        [this, endpoints, reconnectable = std::move(reconnectable)](error_code ec, tcp::endpoint) mutable {
            if(!ec) {
                parent->logger_.info(std::format("Client connected to {}:{}", addr, port));
                // std::unique_lock<std::mutex> lock(mut_conn_finished);
                is_connected = true;
                state.store(Client::ConnectionState::connected);
                // conn_finished.notify_all();
                strand.post([=, this](){
                    for(auto& pending_call : pending_calls) {
                        write(std::move(pending_call));
                    }
                });
                do_read();
            }
            else if(reconnectable()) {
                do_connect(endpoints, std::move(reconnectable));
            }
            else {
                parent->logger_.error(std::format("Error during connection: {} | '{}'", ec.value(), ec.message()));
                // std::unique_lock<std::mutex> lock(mut_conn_finished);
                state.store(Client::ConnectionState::disconnected);
                throw rpc::SystemError(ec);
                // conn_ec = ec;
                // conn_finished.notify_all();
            }
        });
}

void Client::impl::do_connect(endpoints_range endpoints) {
    parent->logger_.info("Initialting connection.");
    // conn_ec = std::nullopt; 默认初始化

    auto re = reconnectable;

    // 异步执行完毕后 socket打开
    boost::asio::async_connect(writer->socket(), endpoints, 
        [this, endpoints, reconnectable = std::move(re)](error_code ec, tcp::endpoint) mutable {
            if(!ec) {
                parent->logger_.info(std::format("Client connected to {}:{}", addr, port));
                // std::unique_lock<std::mutex> lock(mut_conn_finished);
                is_connected = true;
                state.store(Client::ConnectionState::connected);
                // conn_finished.notify_all();
                strand.post([=, this](){
                    for(auto& pending_call : pending_calls) {
                        write(std::move(pending_call));
                    }
                });
                do_read();
            }
            else if(reconnectable()) {
                do_connect(endpoints, std::move(reconnectable));
            }
            else {
                parent->logger_.error(std::format("Error during connection: {} | '{}'", ec.value(), ec.message()));
                // std::unique_lock<std::mutex> lock(mut_conn_finished);
                state.store(Client::ConnectionState::disconnected);
                throw rpc::SystemError(ec);
                // conn_ec = ec;
                // conn_finished.notify_all();
            }
        });
}

void Client::impl::do_read() {
    parent->logger_.trace("do_read");
    std::cout << "do_read" << std::endl;
    constexpr size_t max_read_bytes = default_buffer_size;
    writer->socket().async_read_some(buffer(unpac.buffer(), max_read_bytes),
    // 原作者认为这里没必要捕获 因为max_read_bytes是constexpr
    [this, max_read_bytes](error_code ec, size_t lenth){
        if(!ec) {
            parent->logger_.info(std::format("Read chunk of size {}", lenth));

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
                parent->logger_.trace(std::format("Reserving extra buffer: {}", max_read_bytes));
                unpac.reserve_buffer(max_read_bytes);
            }
            do_read();
        }
        else if(ec == boost::asio::error::eof) {
            parent->logger_.warning("The server closed the connection.");
            state = Client::ConnectionState::disconnected;
        }
        else if(ec == boost::asio::error::connection_reset) {
            // ?
            // Yes, this should be connection_state::reset,
            // but on windows, disconnection results in reset. May be
            // asio bug, may be a windows socket pecularity. Should be
            // investigated later.
            parent->logger_.warning("The server was reset or disconencted");
            state = Client::ConnectionState::disconnected;
        }
        else {
            parent->logger_.error(std::format("Unhandled error code: {} | '{}'", ec.value(), ec.message()));
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
// void Client::wait_conn() {
//     std::unique_lock<std::mutex> lock(pimpl_->mut_conn_finished);
//     // 防止假唤醒
//     while(pimpl_->is_connected == false) {
//         // 还要判断这个 用while挺好的
//         // 否则可以使用带 Predicate 的重载
//         auto ec = pimpl_->conn_ec;
//         if(ec.has_value()) {
//             throw rpc::SystemError(ec.value());
//         }

//         auto timeout_val = pimpl_->timeout_val;
//         if(timeout_val.has_value()) {
//             auto wait_ret = pimpl_->conn_finished.wait_for(lock, 
//                 std::chrono::milliseconds(timeout_val.value()));
//             if(wait_ret == std::cv_status::timeout) {
//                 throw rpc::Timeout(std::format("Timeout of {}ms while connecting to {} | {}", 
//                     timeout_val.value(),pimpl_->addr, pimpl_->port));
//             }
//         }
//         else {
//             pimpl_->conn_finished.wait(lock);
//         }
//     }
// }

void Client::post(std::shared_ptr<msgpack::sbuffer> buffer) {
    // 隐式捕获了this
    // 保证有序的申请写
    pimpl_->strand.post([=, this](){
        pimpl_->write(std::move(*buffer));
    });
}

// 真正的rpc通信还是只发送了sbuffer -- 序列化后的数据
void Client::post(std::shared_ptr<msgpack::sbuffer> buffer, int idx, 
    const std::string& func_name, std::shared_ptr<rpc_promise> p) {
        pimpl_->strand.post([=, this](){
            pimpl_->ongoing_calls[idx] = std::make_pair(func_name, std::move(*p));
            if(pimpl_->is_connected) {
                for(auto& pending_call : pimpl_->pending_calls) {
                    pimpl_->write(std::move(pending_call));
                }
                pimpl_->write(std::move(*buffer));
            }
            else {
                pimpl_->pending_calls.push_back(std::move(*buffer));
            }
        });
}

Client::Client(const std::string& addr, uint16_t port, std::function<bool()> reconnectable)
    :pimpl_(std::make_unique<impl>(this, addr, port, std::move(reconnectable))) {
        tcp::resolver resolver(pimpl_->io);
        // 返回的是个range 但也可以当iter用
        auto endpoints = resolver.resolve(pimpl_->addr, std::to_string(pimpl_->port));
        pimpl_->do_connect(endpoints);
        std::thread io_thread([this](){
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