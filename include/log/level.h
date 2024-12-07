#ifndef LEVEL_H_RPC_LOG
#define LEVEL_H_RPC_LOG

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>

namespace logging {
// 枚举类型 别当成class了 里面的枚举元素就是要用的
// 这里支持大于小于号是语言规定吗？
enum class Level : int8_t {
    Critical = 0,
    Error = 1,
    Warning = 2,
    Info = 3,
    Debug = 4,
    Trace = 5
};

// 将Level枚举转换成文本
// 注意返回的是const std::string& 用引用接一下好点
inline const std::string& to_level_name(Level level) {
    static std::vector<std::string> level_names {
        "CRITICAL",
        "ERROR",
        "WARNING",
        "INFO",
        "DEBUG",
        "TRACE"
    };
    // 枚举可以用static_cast显示转换成对应的底层数据
    // 要返回const引用 不能用operator []
    return level_names.at(static_cast<int8_t>(level));
}

inline std::ostream& operator<<(std::ostream& os, Level level) {
    os << to_level_name(level);
    return os;
}
}


#endif