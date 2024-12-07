#ifndef DISPATCHER_H_RPC
#define DISPATCHER_H_RPC

#include <functional>
#include <memory>
#include <msgpack.hpp>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <source_location>

#include "detail/func_traits.h"
#include "log/logger.h"
#include "detail/response.h"

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
    enum class RequestType{call = 0, notification = 2};

    std::unordered_map<std::string, adaptor_type> funcs_;
    // 需要在LoggerFactory后加上<>以应用默认的模板参数
    logging::DefaultLogger logger_ = logging::LoggerFactory<>::create_logger("Dispatcher");
public:
    template <typename Func>
    void bind(const std::string& name, Func func);
    // 以下是对bind的标签分发
    template <typename Func>
    void bind(const std::string& name, Func func,
        const tags::void_result&,
        const tags::zero_arg&);
    template <typename Func>
    void bind(const std::string& name, Func func,
        const tags::void_result&,
        const tags::nonzero_arg&);
    template <typename Func>
    void bind(const std::string& name, Func func,
        const tags::nonvoid_result&,
        const tags::zero_arg&);
    template <typename Func>
    void bind(const std::string& name, Func func,
        const tags::nonvoid_result&,
        const tags::nonzero_arg&);
    
    void unbind(const std::string& name);

    //! \brief returns a list of all names which functors are binded to
    std::vector<std::string> names() const;

    // 处理msg并执行对应的函数
    void dispatch(const msgpack::sbuffer& msg);

    Response dispatch(const msgpack::object& msg, bool suppress_exception = false);
private:
    //! \brief Checks the argument count and throws an exception if
    //! it is not the expected amount.
    // 跟成员变量都没关系 - 声明为static
    static void enforce_arg_count(const std::string& func_name, size_t found, size_t excepted);

    void enforce_unique_name(const std::string& func_name) const;

    Response dispatch_call(const msgpack::object msg, bool suppress_exception = false);

    Response dispatch_notification(const msgpack::object& msg, bool suppress_exception = false);

    // 这个pack声明应该不需要
};
}

#include "dispatcher.inl"


#endif