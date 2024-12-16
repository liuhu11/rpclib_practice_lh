#ifndef SERVER_H_RPC
#define SERVER_H_RPC

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "dispatcher.h"
#include "log/logger.h"
#include "config.h"

namespace rpc {
namespace detail {
class ServerSession;
}

class Server {
private:
    struct impl;
    std::unique_ptr<impl> pimpl_;
    std::shared_ptr<detail::Dispatcher> disp_;
    static logging::DefaultLogger logger_;
public:
    //! \brief Constructs a server that listens on the localhost on the
    //! specified port.
    explicit Server(uint16_t port);
    Server(Server &&other) noexcept;
    Server(const std::string& address, uint16_t port);
    ~Server();
    Server& operator=(Server &&other);

    void run();
    void async_run(size_t worker_threads = 1);

    template <typename Func>
    void bind(const std::string& name, Func func) {
        disp_->bind(name, func);
    }

    void unbind(const std::string& name);

    std::vector<std::string> names() const;

    void suppress_exceptions(bool suppress);

    //! \brief Stops the server.
    //! \note This should not be called from worker threads.
    void stop();

    uint16_t port() const;

    void close_sessions();
    
    // 指针的引用？
    void close_session(std::shared_ptr<detail::ServerSession> s);
};
}


#endif