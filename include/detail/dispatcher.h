#ifndef DISPATCHER_H_RPC
#define DISPATCHER_H_RPC

#include <functional>
#include <memory>
#include <msgpack.hpp>
#include <string>
#include <tuple>
#include <unordered_map>

#include "log.h"

namespace rpc::detail {
//! \brief This class maintains a registry of functors associated with their
//! names, and callable using a msgpack-rpc call pack.
// 本意是调度器或者分配器
class Dispatcher {
public:
    using adaptor_type = std::function<std::unique_ptr<msgpack::object_handle>(const msgpack::object&)>;
    // 消息类型id 消息id 方法名 传递给方法的参数
    using call_t = std::tuple<int8_t, int32_t, std::string, msgpack::object>;
    // 消息类型id 方法名 传递给方法的参数
    using notification_t = std::tuple<int8_t, std::string, msgpack::object>;
private:
    std::unordered_map<std::string, adaptor_type> funcs_;
    RPC_CREATE_LOG_CHANNEL(Dispatcher)
public:
    template <typename Func>
    void bind(const std::string& name, Func func);
    // 一下是对bind的标签分发
    
};
}


#endif