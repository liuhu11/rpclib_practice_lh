namespace rpc {
template<typename... Args>
msgpack::object_handle Client::call(const std::string& func_name, Args... args) {
    auto future = async_call(func_name, std::forward<Args>(args)...);
    auto timeout_val = timeout();
    if(timeout_val.has_value()) {
        auto wait_ret = future.wait_for(std::chrono::milliseconds(timeout_val.value()));
        if(wait_ret == std::future_status::timeout) {
            throw_timeout(func_name);
        }
    }
    return future.get();
}

template <typename... Args>
std::future<msgpack::object_handle> Client::async_call(const std::string &func_name, Args... args)
{
    // wait_conn();
    logger_.debug(std::format("Calling {}", func_name));

    auto args_tuple = std::make_tuple(args...);
    const int idx = next_call_idx();
    // 为什么要用uint8_t
    auto call_obj = std::make_tuple(static_cast<int8_t>(Client::RequestType::call), 
        idx, func_name, args_tuple);
    auto buffer = std::make_shared<msgpack::sbuffer>();
    msgpack::pack(*buffer, call_obj);


    // Change to move semantics when asio starts supporting move-only handlers in post().
    // 就是这里影响的 ！
    auto promise = std::make_shared<std::promise<msgpack::object_handle>>();
    auto future = promise->get_future();

    post(buffer, idx, func_name, promise);
    return future;
}

template <typename... Args>
void Client::notify(const std::string& func_name, Args... args) {
    logger_.debug(std::format("Sending notification {}.", func_name));

    auto args_tuple = std::make_tuple(args...);
    auto call_obj = std::make_tuple(static_cast<int8_t>(Client::RequestType::notification),
        func_name, args_tuple);

    // 为什么不用智能指针？
    auto buffer = std::make_shared<msgpack::sbuffer>();
    msgpack::pack(*buffer, call_obj);

    post(buffer);
}
}