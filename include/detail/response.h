#ifndef RESPONSE_H_RPC
#define RESPONSE_H_RPC

#include <cstdint>
#include <memory>
#include <msgpack.hpp>
#include <tuple>

#include "log/logger.h"

namespace rpc::detail{
class Response {
private:
    uint32_t id_;
    // 原作者很想避免使用shared_ptr
    // 现在已经支持了 todo
    // but at this point asio does not work with move-only handlers in post() and I need to 
    // capture responses in lambdas.
    std::shared_ptr<msgpack::object_handle> error_;
    std::shared_ptr<msgpack::object_handle> result_;
    bool empty_;
    logging::DefaultLogger logger_;
public:
    // 协议版本，响应id，错误对象，结果对象
    using response_type = std::tuple<uint32_t, uint32_t, msgpack::object, msgpack::object>;
    Response(msgpack::object_handle &&obj);
    // T Any msgpack-able type.
    // If there is both an error and result in the response,
        // the result will be discarded while packing the data.
    template <typename T>
    static Response make_result(uint32_t id, T &&result);
    template <typename T>
    static Response make_error(uint32_t id, T &&error);

    // Gets the response data as a RPCLIB_MSGPACK::sbuffer.
    // 得到序列化后的数据
    // sbuffer通常用以存储序列化后的数据
    msgpack::sbuffer data() const;
    void capture_result(msgpack::object_handle &&result);
    void capture_error(msgpack::object_handle &&error);

    uint32_t id() const;
    std::shared_ptr<msgpack::object_handle> error() const;
    std::shared_ptr<msgpack::object_handle> result() const;

    // Gets an empty response which means "no response" (not to be
        // confused with void return, i.e. this means literally
        // "don't write the response to the socket")
    static Response empty();
    bool is_empty() const;
private:
    Response();
};
template <typename T>
Response Response::make_result(uint32_t id, T &&result) {
    auto zone = std::make_unique<msgpack::zone>();
    msgpack::object obj(std::forward<T>(result), *zone);

    Response response;
    response.id_ = id;

    response.result_ = std::make_shared<msgpack::object_handle>(obj, std::move(zone));
    return response;
}

template <>
inline Response Response::make_result(uint32_t id, std::unique_ptr<msgpack::object_handle> &&result) {
    Response response;
    response.id_ = id;
    // 用uniqe_ptr&& 构造是shared_ptr的构造函数 而不是msgpack::object_handle的 弄清楚
    // 直接 = 也行
    response.result_ = std::shared_ptr<msgpack::object_handle>(std::move(result));
    return response;
}

// 保留了异常类型 -- 但是用的时候都直接传字符串了 -- 要求异常类型可以序列化
template <typename T>
Response Response::make_error(uint32_t id, T &&error) {
    auto zone = std::make_unique<msgpack::zone>();
    msgpack::object obj(std::forward<T>(error), *zone);

    Response response;
    response.id_ = id;
    response.error_ = std::make_shared<msgpack::object_handle>(obj, std::move(zone));
    return response;
}
}

#endif