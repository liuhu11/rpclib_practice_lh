#ifndef THREAD_GROUP_H_RPC
#define THREAD_GROUP_H_RPC

#include <functional>
#include <thread>
#include <vector>

namespace rpc::detail {
class ThreadGroup {
private:
    std::vector<std::thread> threads_;
public:
    ThreadGroup(){}
    ThreadGroup(const ThreadGroup&) = delete;
    ~ThreadGroup() {
        join_all();
    }

    void create_threads(size_t thread_count, std::function<void()> func) {
        for(size_t i = 0; i != thread_count; ++i) {
            threads_.push_back(std::thread(func));
        }
    }

    void join_all() {
        for(auto& thread : threads_) {
            if(thread.joinable()) {
                thread.join();
            }
        }
    }
};
}


#endif