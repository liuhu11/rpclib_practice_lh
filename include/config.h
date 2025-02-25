#ifndef CONFIG_H_RPC
#define CONFIG_H_RPC

#include <cstdint>

namespace rpc{
    // std::intptr_t -- 足以存储指针的整数
    // 用session的this指针的值作为session_id
using session_id_t = std::intptr_t;

struct Constants final{
    static constexpr std::size_t DEFAULT_BUFFER_SIZE = 1024 << 10;
    static constexpr std::uint16_t DEFAULT_PORT = 8080;
};    
}

#endif