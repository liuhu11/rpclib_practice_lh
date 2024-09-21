#ifndef RPC_ERROR_H_RPC
#define RPC_ERROR_H_RPC

#include <memory>
#include <msgpack.hpp>
#include <stdexcept>
#include <string>

namespace rpc {
class RpcError : public std::runtime_error {
private:
    std::string func_name_;
    std::shared_ptr<msgpack::object_handle> obj_handle_;
public:
    RpcError(const std::string& what_arg, const std::string& func_name, 
        std::shared_ptr<msgpack::object_handle> obj_handle);
    std::string func_name() const;
    virtual msgpack::object_handle& error();
};

class Timeout : public std::runtime_error {
private:
    std::string formatted;
public:
    explicit Timeout(const std::string& what_arg);

    const char* what() const noexcept override;
};

class SystemError : public std::system_error {
public:
    using std::system_error::system_error;
    const char* what() const noexcept;
};
}

#endif