#include <atomic>
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <mutex>
#include <ranges>

#include "detail/dev_utils.h"
#include "dispatcher.h"
#include "server.h"
#include "detail/server_session.h"
#include "this_server.h"
#include "detail/thread_group.h"

using boost::asio::io_context;
using boost::asio::ip::address;
using boost::asio::ip::tcp;
using boost::system::error_code;
using rpc::detail::ServerSession;
using rpc::detail::ThreadGroup;
namespace ranges = std::ranges;


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
    std::mutex sessions_mutex;
public:
    impl(Server* parent_arg, const std::string& address, uint16_t port):parent(parent_arg),
        // 这里的acceptor只是被构造了 但还没打开
        io(), acceptor(io), socket(io), suppress_exception(false) {
            auto endpoint = tcp::endpoint(address::from_string(address), port);
            acceptor.open(endpoint.protocol());
            // 作用？
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
                parent->logger_.info(std::format("Accepted connection from {}:{}", endpoint.address().to_string(), endpoint.port()));
                auto session = std::make_shared<ServerSession>(parent, &io, std::move(socket),
                    parent->disp_, suppress_exception);
                session->start();
                std::unique_lock<std::mutex> lock(sessions_mutex);
                sessions.push_back(session);
            }
            else {
                parent->logger_.error(std::format("Error while accepting connection: {} | '{}'", ec.value(), ec.message()));
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
            // 注意这种方法 - 类似写时复制 -- 将具体的close逻辑放在lock之外了
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

Server::Server(uint16_t port):pimpl_(std::make_unique<impl>(this, port)),
    disp_(std::make_shared<detail::Dispatcher>()), logger_(logging::LoggerFactory<>::create_logger("Server")) {
        logger_.info(std::format("Created server on localhost:{}", port));
        pimpl_->start_accept();
}

// 在进入初始化函数体之前 所有的成员都已经构造好了 
Server::Server(Server &&other) noexcept: logger_(std::move(other.logger_)) {
    // 利用赋值运算符实现移动函数
    pimpl_ = std::move(other.pimpl_);
    disp_ = std::move(other.disp_);
}

Server::Server(const std::string& address, uint16_t port)
    :pimpl_(std::make_unique<impl>(this, address, port)), 
    disp_(std::make_shared<detail::Dispatcher>()), logger_(logging::LoggerFactory<>::create_logger("Server")){
        logger_.info(std::format("Created server on {}:{}", address, port));
        pimpl_->start_accept();
}

Server::~Server() {
    if(pimpl_.get() != nullptr) {
        pimpl_->stop();
    }
}

Server& Server::operator=(Server &&other) {
    // 自赋值好像是在先delete自己的成员时 会有问题
    // 但shared_ptr unique_ptr本身都是自赋值安全的 也不会有很多多余操作 所以不需要显示的判断自赋值


    // 也可以拷贝后交换代替显示的自赋值检查
    pimpl_ = std::move(other.pimpl_);
    disp_ = std::move(other.disp_);
    logger_ = std::move(other.logger_);
    return *this;
}

void Server::run() {
    pimpl_->io.run();
}

void Server::async_run(size_t worker_threads) {
    pimpl_->loop_workers.create_threads(worker_threads, [this](){
        detail::name_thread("server");
        logger_.info("Starting");
        pimpl_->io.run();
        logger_.info("Exsiting");
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

void Server::close_session(std::shared_ptr<detail::ServerSession> s) {
    // 局部变量和函数参数都是后构造的先析构

    // lock解锁后参数仍然持有一个共享指针
    // 使得session的析构在释放锁之后
    std::unique_lock<std::mutex> lock(pimpl_->sessions_mutex);
    auto iter = ranges::find(pimpl_->sessions, s);
    if(iter != pimpl_->sessions.end()) {
        pimpl_->sessions.erase(iter);
    }
    // session shared pointer is released outside of the mutex
}

}