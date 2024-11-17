namespace rpc {
template <typename T>
void this_handler_t::respond_error(T &&err_obj) {
    error_ = detail::pack(std::forward<T>(err_obj));
    // 抛出此异常以快速从一次dispatch中返回 -- 给server注册的函数用的
    // 直接抛出了一个默认构造的类
    throw detail::handler_error{};
}

template <typename T>
void this_handler_t::respond(T &&resp_obj) {
    resp_ = detail::pack(std::forward<T>(resp_obj));
    throw detail::handler_sepc_response{};
}
}