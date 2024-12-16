#include <cstdint>

#include <boost/system/error_code.hpp>

#include "config.h"
#include "server.h"
#include "detail/server_session.h"
#include "this_handler.h"
#include "this_server.h"
#include "this_session.h"

using boost::asio::buffer;
using boost::system::error_code;
using msgpack::unpacked;

namespace rpc::detail {
logging::DefaultLogger ServerSession::logger_{logging::LoggerFactory<>::create_logger("ServerSession")};

static constexpr size_t default_buffer_size = rpc::Constants::DEFAULT_BUFFER_SIZE;

ServerSession::ServerSession(Server* srv, boost::asio::io_context* io, 
    boost::asio::ip::tcp::socket&& socket, std::shared_ptr<Dispatcher> disp, bool suppress_exceptions):
        AsyncWriter(io, std::move(socket)), parent_(srv), io_(io), 
        read_strand_(*io), disp_(disp), unpac_(),  suppress_exceptions_(suppress_exceptions) {
            logger_.trace("in ServerSession::ServerSession");
            unpac_.reserve_buffer(default_buffer_size);
}

void ServerSession::start() {
    do_read();
}

void ServerSession::close() {
    logger_.info("Closing session.");
    AsyncWriter::close();

    auto self(shared_from_base<ServerSession>());
    write_strand().post([this, self](){
        // 如果仅仅只有Server类型的声明 是不能调用的
        parent_->close_session(self);
    });
}

void ServerSession::do_read() {
    auto self(shared_from_base<ServerSession>());
    constexpr size_t max_read_bytes = default_buffer_size;

    // wrap()将一个handler包装到strand中
    socket().async_read_some(buffer(unpac_.buffer(), default_buffer_size), 
        read_strand_.wrap([this,self, max_read_bytes](error_code ec, size_t length){
            if(is_closed()) {
                return ;
            }
            if(!ec) {
                // After copying the data to the memory that is pointed by buffer(), 
                // you need to call the function to notify how many bytes are consumed. 
                // Then you can call next() functions.
                unpac_.buffer_consumed(length);

                // typedef msgpack::v1::object_handle msgpack::v1::unpacked
                unpacked result;
                while(unpac_.next(result) && !is_closed()) {
                    auto msg_obj = result.get();
                    output_buf_.clear();
                    // any worker thread can take this call
                    // 使用shared_ptr管理zone
                    std::shared_ptr<msgpack::zone> z(result.zone().release());
                    io_->post([this, self, msg_obj, z]() {
                        this_handler().clear();
                        this_session().clear();
                        this_session().id(reinterpret_cast<session_id_t>(this));
                        this_server().cancel_stop();

                        auto resp = disp_->dispatch(msg_obj, suppress_exceptions_);

                        // There are various things that decide what to send
                        // as a response. They have a precedence.

                        // First, if the response is disabled, that wins
                        // So You Get Nothing, You Lose! Good Day Sir!
                        if(!this_handler().resp_enabled_) {
                            return ;
                        }

                        // Second, if there is an error set, we send that
                        // and third, if there is a special response, we
                        // use it
                        if(!this_handler().error_.get().is_nil()) {
                            logger_.warning("There was an error set in the handler.");
                            resp.capture_error(std::move(this_handler().error_));
                        }
                        else if(!this_handler().resp_.get().is_nil()) {
                            logger_.warning("There wan a special result set in the handler.");
                            resp.capture_result(std::move(this_handler().resp_));
                        }

                        if(!resp.is_empty()) {
                            // 此处不能引用捕获 会有悬垂引用
                            write_strand().post([this, self, resp, z]() {
                                write(resp.data());
                            });
                        }

                        if(this_session().exit_) {
                            logger_.warning("Session exit requested from a handler.");
                            // posting through the strand so this comes after
                            // the previous write
                            write_strand().post([this]() {
                                close();
                            });
                        }

                        if(this_server().stopping()) {
                            logger_.warning("Server exit requested from a handler.");
                            write_strand().post([this]() {
                                parent_->close_sessions();
                            });
                        }
                    });
                }

                if(!is_closed()) {
                    // resizing strategy: if the remaining buffer size is
                    // less than the maximum bytes requested from asio,
                    // then request max_read_bytes. This prompts the unpacker
                    // to resize its buffer doubling its size
                    // (https://github.com/msgpack/msgpack-c/issues/567#issuecomment-280810018)
                    if(unpac_.buffer_capacity() < max_read_bytes) {
                        logger_.trace(std::format("Reserving extra buffer: {}", max_read_bytes));
                        unpac_.reserve_buffer(max_read_bytes);
                    }
                    do_read();
                }
            }
            else if(ec == boost::asio::error::eof ||
                        ec == boost::asio::error::connection_reset) {
                        logger_.info("Client disconnection");
                        self->close();
                    }
            else {
                logger_.error(std::format("Unhandled error code: {} | '{}'", ec.value(), ec.message()));
            }
        }));
}
}