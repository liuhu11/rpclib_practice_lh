#include <msgpack.hpp>
#include <tuple>
#include <cstdint>
#include <memory>

#include "detail/response.h"

int main() {
    auto z = std::make_unique<msgpack::zone>();
    auto res = msgpack::object(std::vector<std::string>{"13", "22"}, *z);
    auto p = std::make_unique<msgpack::object_handle>(res, std::move(z));

    auto resp = rpc::detail::Response::make_result(1, std::move(p));

    auto data = resp.data();

    return 0;
}