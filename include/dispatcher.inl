namespace rpc::detail {
template <typename Func>
void Dispatcher::bind(const std::string& name, Func func) {
    enforce_unique_name(name);

    // 这里也是用到的类型然后初始化 需要加typename （调用模板函数也要加template）
    bind(name, func, typename func_kind_info<Func>::result_kind{}, 
        typename func_kind_info<Func>::args_kind{});
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
            // object_handle不支持复制和移动 -- 所以返回unique_ptr
            return std::make_unique<msgpack::object_handle>();
        };
}

template <typename Func>
void Dispatcher::bind(const std::string& name, Func func,
    const tags::void_result&,
    const tags::nonzero_arg&) {
        enforce_unique_name(name);
        // 这里lambda也可见
        using args_type = typename func_traits<Func>::args_type;
        // 如果变量满足下列条件之一，那么 lambda 表达式在读取它的值前不需要先捕获：
            // 该变量具有 const 而非 volatile 的整型或枚举类型，并已经用常量表达式初始化。
            // 该变量是 constexpr 的且没有 mutable 成员。
        constexpr size_t args_count = func_traits<Func>::arg_count::value;

        funcs_[name] = [name, func](const msgpack::object& args) {
            // 这里是Args的tuple形式
            enforce_arg_count(name, args.via.array.size, args_count);
            args_type args_real;
            args.convert(args_real);
            std::apply(func, args_real);
            return std::make_unique<msgpack::object_handle>();
        };
}

template <typename Func>
void Dispatcher::bind(const std::string& name, Func func,
    const tags::nonvoid_result&,
    const tags::zero_arg&) {
        enforce_unique_name(name);

        funcs_[name] = [func, name](const msgpack::object& args) {
            enforce_arg_count(name, args.via.array.size, 0);

            // 此处用unique_ptr是因为object_handle的构造函数需要unique_ptr<zone>&&
            auto z = std::make_unique<msgpack::zone>();
            auto res = msgpack::object(func(), *z);
            return std::make_unique<msgpack::object_handle>(res, std::move(z));
        };
}

template <typename Func>
void Dispatcher::bind(const std::string& name, Func func,
    const tags::nonvoid_result&,
    const tags::nonzero_arg&) {
        enforce_unique_name(name);

        using args_type = typename func_traits<Func>::args_type;
        constexpr size_t arg_count = func_traits<Func>::arg_count::value;

        funcs_[name] = [name, func](const msgpack::object& args) {
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