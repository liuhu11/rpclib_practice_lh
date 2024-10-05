#ifndef UTIL_H_RPC
#define UTIL_H_RPC

#include <memory>
#include <msgpack.hpp>

namespace rpc::detail {
// 万能引用
template <typename T>
msgpack::object_handle pack(T &&obj) {
    auto z = std::make_unique<msgpack::zone>();
    msgpack::object obj(std::forward<T>(obj), *z);
    return msgpack::object_handle(obj, std::move(z));
}
}

#endif