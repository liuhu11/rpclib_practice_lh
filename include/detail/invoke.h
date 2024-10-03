#ifndef INVOKE_H_RPC
#define INVOKE_H_RPC

namespace rpc::detail {
template <typename T>
using invoke_type = typename T::type;
}


#endif