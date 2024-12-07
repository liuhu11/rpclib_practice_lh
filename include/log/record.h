#ifndef RECORD_H_RPC_LOG
#define RECORD_H_RPC_LOG


#include "log/level.h"
#include <chrono>
#include <source_location>

namespace logging {
// 就用chrono
using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

struct Record {
    // logger_name
    std::string name;
    Level level;
    TimePoint time;
    std::string msg;
    std::source_location source_location;

    const std::string& level_name() const {
        return to_level_name(level);
    }

};
}


#endif