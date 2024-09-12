#include "response.h"

using msgpack::object;
using msgpack::object_handle;
using msgpack::sbuffer;

namespace rpc::detail {
Response::Response():id_(0), empty_(false) {}

Response::Response(object_handle o):Response() {
    response_type response;
    // o.get()返回object的常引用
    // convert()用于转换数据
    o.get().convert(response);

    id_ = std::get<1>(response);
    auto &&err_obj = std::get<2>(response);
    if(!err_obj.is_nil()) {
        error_ = std::make_shared<object_handle>();
        *error_ = msgpack::clone(err_obj);
    }
    result_ = std::make_shared<object_handle>(std::get<3>(response), std::move(o.zone()));
}

sbuffer Response::data() const {
    sbuffer data;

    // 注意.get()与->get()的不同
    // 前者调用shared_ptr的方法，后者调用存储的object_handle的方法
    response_type response(1, id_, error_.get() == nullptr ? object() : error_->get(), 
        result_.get() == nullptr ? object() : result_->get());
    // 将response序列化到data中
    msgpack::pack(data, response);
    return data;
}

void Response::capture_result(msgpack::object_handle &&result) {
    if(result_.get() == nullptr) {
        result_ = std::make_shared<object_handle>();
    }
    // 其实还是复制
    result_->set(std::move(result).get());
}

void Response::capture_error(msgpack::object_handle &&error) {
    if(error_.get() == nullptr) {
        error_ = std::make_shared<object_handle>();
    }
    error_->set(std::move(error).get());
}

uint32_t Response::id() const {
    return id_;
}

std::shared_ptr<msgpack::object_handle> Response::error() const {
    return error_;
}

std::shared_ptr<msgpack::object_handle> Response::result() const {
    return result_;
}

Response Response::empty() {
    Response response;
    response.empty_ = true;
    return response;
}

bool Response::is_empty() const {
    return empty_;
}
}