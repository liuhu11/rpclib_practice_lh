#include "server.h"

#include <thread>
#include <set>
#include <chrono>
#include <cstdint>
#include <format>
#include <vector>
#include <iostream>
#include <mutex>
#include <syncstream>

class Master {
public:
    void subscribe(std::string id) {
        update_time();
        clients_.insert(id);
    }

    std::vector<std::string> list() {
        update_time();
        return std::vector<std::string>(clients_.cbegin(), clients_.cend());
    }

    uint32_t idle_time() {
        auto now = std::chrono::system_clock::now();
        {
            std::lock_guard<std::mutex> lock(mut_);
            auto diff = now - last_time_; 
            return std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
        }
    }

private:
    void update_time() {
        std::lock_guard<std::mutex> lock(mut_);
        last_time_ = std::chrono::system_clock::now();
    }
    std::mutex mut_;
    std::set<std::string> clients_;
    std::chrono::time_point<std::chrono::system_clock> last_time_ = std::chrono::system_clock::now();
};

int main() {
    rpc::Server srv(rpc::Constants::DEFAULT_PORT);
    Master m;
    constexpr uint32_t MAX_IDLE_TIME = 5000;

    srv.bind("subscribe", [&m](std::string id) {
        m.subscribe(id);
    });

    srv.bind("list", [&m]() {
        std::osyncstream os(std::cout);
        os << "list: " << std::endl;
        for(const auto& str : m.list()) {
            os << str << " "; 
        }
        os << std::endl;
        return m.list();
    });

    srv.async_run();

    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if(m.idle_time() > MAX_IDLE_TIME) {
            srv.stop();
            break;
        }
    }

    {
        // 好像format不能格式化steady_clock的时间点
        auto os = std::osyncstream(std::cout);
        os << std::format("[server] {0:%Y-%m-%d}T{0:%H:%M:%OS}Z - now subscribed clients: ", std::chrono::system_clock::now());
        for(auto id : m.list()) {
            os << id << ",";
        }
        os << std::endl;
    }
    return 0;
}