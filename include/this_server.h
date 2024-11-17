#ifndef THIS_SERVER_H_RPC
#define THIS_SERVER_H_RPC

namespace rpc {
namespace detail {
class ServerSession;
}

//! \brief Allows controlling the server instance from the
//! currently executing handler.
class this_server_t {
private:
    // 类的默认构造函数不会对基本数据类型进行初始化
    bool stopping_ = false;
public:
    //! \brief Gracefully stops the server.
    void stop();

    void cancel_stop();

    bool stopping() const;
};

//! \brief A thread-local object that can be used to control
//! the behavior of the server w.r.t. the handler. Accessing this object
//! from handlers that execute the same function concurrently is safe.
//! \note Accessing this object outside of handlers while a server is
//! running is potentially unsafe.
this_server_t& this_server();
}



#endif