#include <boost/system/error_code.hpp>

#include "detail/async_writer.h"

using boost::asio::async_write;
using boost::asio::buffer;
using boost::asio::io_context;
using boost::asio::ip::tcp;
using boost::system::error_code;
using msgpack::sbuffer;

namespace rpc::detail {
AsyncWriter::AsyncWriter(io_context *io, tcp::socket&& socket):socket_(std::move(socket)),
    write_strand_(*io) {}

void AsyncWriter::close() {
    // 先不管内存序的问题
    // TODO: 内存序
    exit_.store(true);

    // 保证生命周期
    auto self = shared_from_this();
    write_strand_.post([this, self]() {
        LOG_INFO("Closing socket");

        error_code ec;
        socket_.shutdown(tcp::socket::shutdown_both, ec);
        if(ec) {
            LOG_WARN("system_error during socket shutdown. "
                            "Code: {}. Message: {}", e.value(), e.message());
        }
        // 不论shutdown是否成功，close兜底
        socket_.close();
    });
}

bool AsyncWriter::is_closed() const {
    return exit_.load();
}

void AsyncWriter::do_write() {
    if(exit_.load()) {
        return ;
    }

    auto self = shared_from_this();
    auto& item = write_queue_.front();

    // 第三个参数是回调的handler
    async_write(socket_, buffer(item.data(), item.size()), write_strand_.wrap(
        [this, self](const error_code& ec, size_t transferred){
            // 不关心传输了多少
            (void)transferred;
            if(!ec) {
                write_queue_.pop_front();

                // 可以写的话继续异步写
                if(write_queue_.size() > 0 && exit_.load() == false) {
                    do_write();
                }
            }
            else {
                LOG_ERROR("Error while writing to socket: {}", ec)
            }
        }
    ));
}

void AsyncWriter::write(sbuffer &&data) {
    if(exit_.load()) {
        return ;
    }

    write_queue_.push_back(std::move(data));

    if(write_queue_.size() > 1) {
        // 原本写队列就有消息要写，不用启动了
        return ;
    }
    do_write();
}

tcp::socket& AsyncWriter::socket() {
    return socket_;
}

io_context::strand& AsyncWriter::write_strand() {
    return write_strand_;
}
}