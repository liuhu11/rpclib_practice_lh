#ifndef SERVER_SESSION_H_RPC
#define SERVER_SESSION_H_RPC

#include <boost/asio.hpp>
#include <memory>
#include <msgpack.hpp>

#include "detail/async_writer.h"
#include "dispatcher.h"
#include "log/logger.h"

namespace rpc {
class Server;

namespace detail {
class ServerSession : public AsyncWriter {
private:
    // 不是所有权的关系 - 先用裸指针
    Server* parent_;
    boost::asio::io_context *io_;
    // boost::asio::io_context::strand 和 boost::asio::strand的区别
    boost::asio::io_context::strand read_strand_;
    std::shared_ptr<Dispatcher> disp_;
    msgpack::unpacker unpac_;
    // sbuffer通常用以存储序列化后的数据
    msgpack::sbuffer output_buf_;
    const bool suppress_exceptions_;
    logging::DefaultLogger logger_;
public:
    ServerSession(Server* srv, boost::asio::io_context* io, boost::asio::ip::tcp::socket&& socket, 
        std::shared_ptr<Dispatcher> disp, bool suppress_exceptions);
    void start();
    void close();
private:
    void do_read();
};
}
}


#endif