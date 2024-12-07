#include "log/formatter.h"
#include "log/record.h"

#include <format>
#include <exception>
#include <iostream>
#include <source_location>

namespace logging::formatters {
std::string format_record(const logging::Record& record) {
    try {
        // 0:<16 第0个提供的参数 <指左对齐 16指至少16位 默认用空格填充
        // regex用普通字符串是要考虑[]的转义的
        // 重用第二个参数 是一定要显示带上是第几个的
        // format支持格式化时间
        return std::format( "{0:<16}| [{1}] {2:%Y-%m-%d}T{2:%H:%M:%OS}Z - <{3}:{4} [{5}]> - {6}", 
            record.name, record.level_name(), record.time, record.source_location.file_name(), record.source_location.line(), 
                record.source_location.function_name(), record.msg);
    }
    catch (std::exception& e) {
        std::cerr << "Error in format: " << e.what() << std::endl;

        throw;
    }
}
}
