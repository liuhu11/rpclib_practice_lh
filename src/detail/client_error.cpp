#include <cstdint>
#include <format>

#include "detail/client_error.h"

namespace rpc::detail {
ClientError::ClientError(code c, const std::string& msg):
    // 0和1是占位符，尤其在需要重用某些参数时需要使用1
    // :04x表示有前导0，4位，16进制
    what_(std::format("client error C{0:04x}: {1}", static_cast<uint16_t>(c), msg)) {}
const char* ClientError::what() const noexcept {
    return what_.c_str();
}
}