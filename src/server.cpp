#include <atomic>
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <mutex>

#include "dev_utils.h"
#include "dispatcher.h"
#include "log.h"
#include "server.h"
#include "server_session.h"
#include "this_server.h"
#include "thread_group.h"

using boost::asio::io_context;
using boost::asio::ip::address;
using boost::asio::ip::tcp;
using boost::system::error_code;
using rpc::detail::ServerSession;
using rpc::detail::ThreadGroup;


namespace rpc {
struct Server::impl {
public:
    Server* parent;
    io_context io;
    tcp::acceptor acceptor;
    tcp::socket socket;
    ThreadGroup loop_workers;
    std::vector<std::shared_ptr<ServerSession>> sessions;
    std::atomic<bool> suppress_exception;
    RPC_CREATE_LOG_CHANNEL(Server)
    std::mutex sessions_mutex;
public:
    impl(Server* parent_arg, const std::string& address, uint16_t port):parent(parent_arg),
        io(), acceptor(io), socket(io), suppress_exception(false) {
            auto endpoint = tcp::endpoint(address::from_string(address), port);
            acceptor.open(endpoint.protocol());
            acceptor.set_option(tcp::acceptor::reuse_address(true));
            acceptor.bind(endpoint);
            acceptor.listen();
    }

    impl(Server* parent_arg, uint16_t port):parent(parent_arg), io(),
        acceptor(io), socket(io), suppress_exception(false) {
            auto endpoint = tcp::endpoint(tcp::v4(), port);
            acceptor.open(endpoint.protocol());
            acceptor.set_option(tcp::acceptor::reuse_address(true));
            acceptor.bind(endpoint);
            acceptor.listen();
    }

    void start_accept() {
        // 注意socket作为新的连接的tcp套接字
        acceptor.async_accept(socket, [this](error_code ec) {
            if(!ec) {
                auto endpoint = socket.remote_endpoint();
                LOG_INFO("Accepted connection from {}:{}", endpoint.address(), endpoint.port());
                auto session = std::make_shared<ServerSession>(parent, &io, std::move(socket),
                    parent->disp_, suppress_exception);
                session->start();
                std::unique_lock<std::mutex> lock(sessions_mutex);
                sessions.push_back(session);
            }
            else {
                LOG_ERROR("Error while accepting connection: {}", ec)
            }
            if(!this_server().stopping()) {
                start_accept();
            }
            // TODO: allow graceful exit [sztomi 2016-01-13]
        });
    }

    void close_sessions() {
        std::vector<std::shared_ptr<ServerSession>> sessions_copy;
        {
            // 注意这种方法 - 写时复制
            std::unique_lock<std::mutex> lock(sessions_mutex);
            sessions_copy = sessions;
            sessions.clear();
        }
        for(auto& session : sessions_copy) {
            session->close();
        }
        if(this_server().stopping()) {
            acceptor.cancel();
        }
    }

    void stop() {
        io.stop();
        loop_workers.join_all();
    }

    uint16_t port() const {
        return acceptor.local_endpoint().port();
    }
};

RPC_CREATE_LOG_CHANNEL(Server)

Server::Server(uint16_t port):pimpl_(std::make_unique<impl>(this, port)),
    disp_(std::make_shared<detail::Dispatcher>()) {
        LOG_INFO("Created server on localhost:{}", port)
        pimpl_->start_accept();
}

Server::Server(Server &&other) noexcept {
    // 利用赋值运算符实现移动函数
    *this = std::move(other);
}

Server::Server(const std::string& address, uint16_t port)
    :pimpl_(std::make_unique<impl>(this, address, port)), 
    disp_(std::make_shared<detail::Dispatcher>()){
        LOG_INFO("Created server on {}:{}", address, port)
        pimpl_->start_accept();
}

Server::~Server() {
    if(pimpl_.get() != nullptr) {
        pimpl_->stop();
    }
}

Server& Server::operator=(Server &&other) {
    if(this != &other) {
        pimpl_ = std::move(other.pimpl_);
        disp_ = std::move(other.disp_);
    }
    return *this;
}

void Server::run() {
    pimpl_->io.run();
}

void Server::async_run(size_t worker_threads = 1) {
    pimpl_->loop_workers.create_threads(worker_threads, [this](){
        detail::name_thread("server");
        LOG_INFO("Starting")
        pimpl_->io.run();
        LOG_INFO("Exsiting")
    });
}

void Server::unbind(const std::string& name) {
    disp_->unbind(name);
}

std::vector<std::string> Server::names() const {
    return disp_->names();
}

void Server::suppress_exceptions(bool suppress) {
    pimpl_->suppress_exception = suppress;
}

void Server::stop() {
    pimpl_->stop();
}

uint16_t Server::port() const {
    return pimpl_->port();
}

void Server::close_sessions() {
    pimpl_->close_sessions();
}

void Server::close_session(const std::shared_ptr<detail::ServerSession>& s) {
    // 但是参数仍然持有一个共享指针？ - 可以学习下思路！
    // 多给一个指针 使得session的析构在释放锁之后
    std::shared_ptr<detail::ServerSession> session;
    {
        std::unique_lock<std::mutex> lock(pimpl_->sessions_mutex);
        auto iter = std::find(pimpl_->sessions.begin(), pimpl_->sessions.end(), s);
        if(iter != pimpl_->sessions.end()) {
            session = *iter;
            pimpl_->sessions.erase(iter);
        }
    }
    // session shared pointer is released outside of the mutex
}

}