#include "client.h"

#include <thread>
#include <iostream>
#include <random>
#include <format>
#include <chrono>
#include <vector>
#include <string>
#include <tuple>
#include <syncstream>
#include <sstream>

int main() {
    auto test_func = []() {
        // 这里的参数是server的port
        rpc::Client c("10.0.16.4", rpc::Constants::DEFAULT_PORT);
        auto id_str = (std::ostringstream{} << std::this_thread::get_id()).str();
        // notify不会等待连接 可能会在连接还没建立时就写
        std::cout << "call begin" << std::endl;
        c.call("subscribe", id_str);
        std::cout << "async_call begin" << std::endl;
        auto f = c.async_call("list");

        std::osyncstream(std::cout) << "do something else..." << std::endl;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distri(10, 1000);

        uint32_t sleep_duration = distri(gen);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_duration));

        {
            // vector本来也不能cout 也没有特化
            auto os = std::osyncstream(std::cout);
            os << std::format("[client] {0:%Y-%m-%d}T{0:%H:%M:%OS}Z - now subscribed clients: ", std::chrono::system_clock::now());
            auto list = f.get().get().as<std::vector<std::string>>();
            for(auto id : list) {
                os << id << ",";
            }
            os << std::endl;
        }
    };

    try {
        std::vector<std::jthread> threads;
        for(int i = 0; i != 5; ++i) {
            threads.emplace_back(test_func);
        }
    }
    catch(rpc::RpcError& e) {
        {
            auto err = e.error().get().as<std::tuple<int, std::string>>();
            std::osyncstream(std::cout) << std::format("{}\nin function '{}': [error {}]: {}", e.what(), e.func_name(), 
                std::get<0>(err), std::get<1>(err)) << std::endl;
        }
        // 这就相当于是处理了异常
        return 1;
    }
    return 0;
}