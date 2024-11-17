#ifndef THIS_HANDLER_H_RPC
#define THIS_HANDLER_H_RPC

#include <msgpack.hpp>

#include "detail/util.h"

namespace rpc {
namespace detail {
class server_session;
// 这两个类是已经定义了  只不过什么都没有
class handler_error{};
class handler_sepc_response{};
}


// 封装有关当前正在执行的处理程序的信息。
// error_和resp_ 会在友元类被使用 -- detail::server_session
class this_handler_t {
private:
    msgpack::object_handle error_, resp_;
    bool resp_enabled_ = true;
public:
    friend class detail::server_session;
    // 注意是万能引用
    template <typename T>
    void respond_error(T &&err_obj);

    template <typename T>
    void respond(T &&resp_obj);

    void disable_response();
    void enable_response();

    //! \brief Sets all state of the object to default.
    void clear();
};
}

#include "this_handler.inl"

namespace rpc {
//! \brief A thread-local object that can be used to control
//! the behavior of the server w.r.t. the handler. Accessing this object
//! from handlers that execute the same function concurrently is safe.
//! \note Accessing this object outside of handlers while a server is
//! running is potentially unsafe.
this_handler_t& this_handler();
}


#endif