#include <msgpack.hpp>
#include <tuple>
#include <cstdint>
#include <memory>

int main() {
    auto z = std::make_unique<msgpack::zone>();
    auto res = msgpack::object(std::vector<std::string>{"13", "22"}, *z);
    auto p = std::make_shared<msgpack::object_handle>(res, std::move(z));
    std::tuple<uint32_t, uint32_t, msgpack::object, msgpack::object> t(1, 2, msgpack::object{}, p->get());
    msgpack::sbuffer data;
    msgpack::pack(data, t);

    return 0;
}