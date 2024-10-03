#ifndef CONSTANT_H_RPC
#define CONSTANT_H_RPC

#include <type_traits>

namespace rpc::detail {
template <typename T, T I>
struct constant : std::integral_constant<T, I>{};

}

#endif