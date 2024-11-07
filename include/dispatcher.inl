namespace rpc::detail {
template <typename Func>
void Dispatcher::bind(const std::string& name, Func func) {
    enforce_unique_name(name);

    bind(name, func, typename func_kind_of<Func>::result_kind, 
        typename func_kind_of<Func>::args_kind);
}

template <typename Func>
void Dispatcher::bind(const std::string& name, Func func,
    const tags::void_result&,
    const tags::zero_arg&) {
        enforce_unique_name(name);

        funcs_[name] = [func, name](const msgpack::object& args) {
            // via是一个联合体
            // size是array中msgpack数据类型的个数
            enforce_arg_count(name, args.via.array.size, 0);
            func();
            // 为什么要返回unique_ptr？
            return std::make_unique<msgpack::object_handle>();
        };
}

template <typename Func>
void Dispatcher::bind(const std::string& name, Func func,
    const tags::void_result&,
    const tags::nonzero_arg&) {
        enforce_unique_name(name);

        funcs_[name] = [name, func](const msgpack::object& args) {
            using args_type = typename func_traits<func>::args_type;
            constexpr size_t args_count = func_traits<func>::arg_count::value;

            enforce_arg_count(name, args.via.array.size, args_count);
            args_type args_real;
            args.convert(args_real);
            std::apply(func, args_real);
            return std::make_unique<msgpack::object_handle>();
        }
}

template <typename Func>
void bind(const std::string& name, Func func,
    const tags::nonvoid_result&,
    const tags::zero_arg&) {
        enforce_unique_name(name);

        funcs_[name] = [func, name](const msgpack::object& args) {
            enforce_arg_count(name, args.via.array.size, 0);

            auto z = std::make_unique<msgpack::zone>();
            auto res = msgpack::object(func(), *z);
            return std::make_unique<msgpack::object_handle>(res, std::move(z));
        };
}

template <typename Func>
void bind(const std::string& name, Func func,
    const tags::nonvoid_result&,
    const tags::nonzero_arg&) {
        enforce_unique_name(name);

        funcs_[name] = [name, func](const magpak::object& args) {
            using args_type = typename func_traits<func>::args_type;
            constexpr size_t arg_count = func_traits<func>::arg_count::value;
            // 感觉原本的应该是有点小错误的
            enforce_arg_count(name, args.via.array.size, arg_count);
            args_type args_real;
            args.convert(args_real);
            auto z = std::make_unique<msgpack::zone>();
            auto res = msgpack::object(std::apply(func, args_real), *z);
            return std::make_unique<msgpack::object_handle>(res, std::move(z));
        };
}
}