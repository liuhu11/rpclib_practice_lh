#include <format>

#include "rpc_error.h"

namespace rpc {
RpcError::RpcError(const std::string& what_arg, const std::string& func_name, 
    std::shared_ptr<msgpack::object_handle> obj_handle):std::runtime_error(what_arg),
    func_name_(func_name), obj_handle_(std::move(obj_handle)) {}
std::string RpcError::func_name() const {
    return func_name_;
}
msgpack::object_handle& RpcError::error() {
    return *obj_handle_;
}

Timeout::Timeout(const std::string& what_arg):std::runtime_error(what_arg) {
    formatted = std::format("rpc::timeout: {}", std::runtime_error::what());
}

// 此时对象是const的 成员变量也是const的了
const char* Timeout::what() const noexcept {
    return formatted.data();
}

const char* SystemError::what() const noexcept {
    // 子类显式调用父类的相关函数
    return std::system_error::what();
}
}