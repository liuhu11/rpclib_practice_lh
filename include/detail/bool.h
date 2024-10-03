#ifndef BOOL_H_RPC
#define BOOL_H_RPC

#include "constant.h"

namespace rpc::detail {
template <bool B>
using bool_ = constant<bool, B>;

using true_ = bool_<true>;

using false_ = bool_<false>;
}

#endif