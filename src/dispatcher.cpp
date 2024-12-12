#include <algorithm>
#include <format>
#include <iterator>
#include <ranges>
#include <stdexcept>

#include "detail/client_error.h"
#include "dispatcher.h"
#include "this_handler.h"
// 命名空间别名 好像是c++20引入的
namespace ranges = std::ranges;


namespace rpc::detail {
void Dispatcher::enforce_unique_name(const std::string& func_name) const {
    if(funcs_.count(func_name) != 0) {
        throw std::logic_error(std::format("Function name already bound: '{}. Please use unique "
            "function name.", func_name));
    }
}

void Dispatcher::enforce_arg_count(const std::string& func_name, size_t found, size_t excepted) {
    if(found != excepted) {
        throw ClientError(ClientError::code::wrong_arity,
            std::format("Function '{0} was called with an invalid number of arguments. "
                "Excepted: {1}, got: {2}", func_name, excepted, found));
    }
}

void Dispatcher::unbind(const std::string& name) {
    funcs_.erase(name);
}

std::vector<std::string> Dispatcher::names() const {
    std::vector<std::string> ret;
    ranges::transform(funcs_, std::back_inserter(ret), [](const std::string& name) {
        return name;
    }, [](const auto& pa) {return pa.first;});
    return ret;
}

void Dispatcher::dispatch(const msgpack::sbuffer& msg) {
    auto unpacked = msgpack::unpack(msg.data(), msg.size());
    dispatch(unpacked.get());
}

Response Dispatcher::dispatch(const msgpack::object& msg, bool suppress_exception) {
    switch(msg.via.array.size) {
        case 3:
            return dispatch_notification(msg, suppress_exception);
        case 4:
            return dispatch_call(msg, suppress_exception);
        default:
            return Response::empty();
    }
}

Response Dispatcher::dispatch_call(const msgpack::object msg, bool suppress_exception) {
    call_t the_call;
    msg.convert(the_call);

    // todo 验证第0位code 即验证协议

    // 必定是左值，但这里采用了万能引用
    // 应该是引用捕获可以保存 底层const属性
    auto &&id = std::get<1>(the_call);
    auto &&name = std::get<2>(the_call);
    auto &&args = std::get<3>(the_call);

    auto func_iter = funcs_.find(name);
    if(func_iter != funcs_.end()) {
        logger_.debug(std::format("Dispatching call to '{}'", name));
        try {
            auto res = func_iter->second(args);
            return Response::make_result(id, std::move(res));
            logger_.info("func finished");
        }
        // 一般用左值引用捕获异常 -- 可能需要修改异常
        catch(ClientError& e) {
            return Response::make_error(id, std::format("rpclib: {}", e.what()));
        }
        catch(std::exception& e) {
            if(!suppress_exception) {
                throw;
            }
            return Response::make_error(id, std::format("rpclib: function '{0}' (called with {1} "
                "arg(s)) threw an exception. The exception contained this informatin: {2}.", name, 
                args.via.array.size, e.what()));
        }
        catch(handler_error& e) {
            // doing nothing, the exception was only thrown to
            // return immediately
            logger_.info("return by handler_error");
        }
        catch(handler_sepc_response& e) {
            // doing nothing, the exception was only thrown to
            // return immediately
            logger_.info("return by handler_sepc_response");
        }
        catch(...) {
            if(!suppress_exception) {
                throw;
            }
            return Response::make_error(id,
                std::format("rpclib: function '{0}' (called with {1} arg(s)) threw an exception."
                    "No further information available.", name, msg.via.array.size));
        }
    }
    return Response::make_error(id, 
        std::format("rpclib: server could not find function '{0}' wtith argument count {1}.",
            name, msg.via.array.size));

}

Response Dispatcher::dispatch_notification(const msgpack::object& msg, bool suppress_exception) {
    notification_t the_call;
    msg.convert(the_call);

    auto &&name = std::get<1>(the_call);
    auto &&args = std::get<2>(the_call);

    auto func_iter = funcs_.find(name);

    if(func_iter != funcs_.end()) {
        logger_.debug(std::format("Dispatching call to '{}'", name));
        try {
            auto res = func_iter->second(args);

        }
        catch(handler_error& e) {
            // doing nothing, the exception was only thrown to
            // return immediately
        }
        catch(handler_sepc_response& e) {
            // doing nothing, the exception was only thrown to
            // return immediately
        }
        catch(...) {
            if(!suppress_exception) {
                throw;
            }
        }
    }
    return Response::empty();
}
}
