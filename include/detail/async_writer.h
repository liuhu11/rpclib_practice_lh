#ifndef ASYNC_WRITER_H_RPC
#define ASYNC_WRITER_H_RPC

#include <atomic>
#include <boost/asio.hpp>
#include <deque>
#include <memory>
#include <msgpack.hpp>

#include "log/logger.h"

namespace rpc::detail {
// 异步写 包含一个写队列
class AsyncWriter : public std::enable_shared_from_this<AsyncWriter> {
private:
    boost::asio::ip::tcp::socket socket_;
    boost::asio::io_context::strand write_strand_;
    std::atomic<bool> exit_{false};
    std::deque<msgpack::sbuffer> write_queue_;
    static logging::DefaultLogger logger_;
    
public:
    // socket值传递改为右值引用传递
    AsyncWriter(boost::asio::io_context *io, boost::asio::ip::tcp::socket&& socket);
    // 优雅关闭，保证之前的写完
    void close();
    bool is_closed() const;
    // 异步写队列中的第一个元素
    // 回调函数才弹出元素
    void do_write();
    // 压入写的队列
    void write(msgpack::sbuffer &&data);
    boost::asio::ip::tcp::socket& socket();
protected:
    // 用以给子类调用拿到相应的shared_ptr
    template <typename Derived>
    std::shared_ptr<Derived> shared_from_base();
    boost::asio::io_context::strand& write_strand();
};

template <typename Derived>
std::shared_ptr<Derived> AsyncWriter::shared_from_base() {
    return std::static_pointer_cast<Derived>(shared_from_this());
}
}

#endif