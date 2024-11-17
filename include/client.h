#ifndef CLIENT_H_RPC
#define CLIENT_H_RPC

#include <future>
#include <memory>
#include <msgpack.hpp>
#include <optional>
#include <string>

#include "config.h"

namespace rpc{

// 客户端，能连接rpc_server，并支持同步或异步的函数调用
// 在创建时，它会异步连接到指定的服务器，并在对象被销毁时自动断开连接
class Client {
private:
    using rpc_promise = std::promise<msgpack::object_handle>;

    enum class RequestType {
        call = 0,
        notification = 2
    };
    enum class ConnectionState {
        initial,
        connected,
        disconnected,
        reset
    };
private:
    static constexpr double buffer_grow_factor_ = 1.8;
    struct impl;
    std::unique_ptr<impl> pimpl_;
public:
    // 异步连接
    // addr可以使ip地址也可以是域名
    Client(const std::string& addr, uint16_t port);

    Client(const Client&) = delete;

    // 优雅关闭
    // 等待未完成的读写完成
    ~Client();

    // todo: 可得到server中发生的异常 加上 in rpc_server 前缀？
    template<typename... Args>
    msgpack::object_handle call(const std::string& func_name, Args... args);

    template<typename... Args>
    std::future<msgpack::object_handle> async_call(const std::string& func_name, Args... args);

    template<typename... Args>
    void notify(const std::string& func_name, Args... args);

    // 影响同步调用
    // 单位为毫秒
    std::optional<int64_t> timeout() const;

    void timeout(int64_t val);

    void clear_timeout();

    ConnectionState connection_state() const;

    // void wait_all_responses();

private:
    void wait_conn();

    void post(std::shared_ptr<msgpack::sbuffer> buffer, int idx, 
        const std::string& func_name, std::shared_ptr<rpc_promise> p);
    void post(std::shared_ptr<msgpack::sbuffer> buffer);

    int next_call_idx();
    [[noreturn]] void throw_timeout(const std::string& func_name);
};

}

#include "client.inl"

#endif