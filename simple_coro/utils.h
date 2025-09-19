#pragma once


#include <stdexcept>
#include <spdlog/spdlog.h>

inline void logicAssert(bool x, const char *errorMsg,
                        std::source_location source = std::source_location::current()) {
    if (x)[[likely]]{
        return;
    }

    spdlog::info("{}:{}:{} err: {}", source.file_name(), source.line(), source.column(), errorMsg);

    throw std::logic_error(errorMsg);
}

inline void logicAssert(bool x, std::string_view errorMsg,
                        std::source_location source = std::source_location::current()) {
    if (x)[[likely]]{
        return;
    }
    spdlog::info("{}:{}:{} err: {}", source.file_name(), source.line(), source.column(), errorMsg);

    throw std::logic_error(errorMsg.data());
}
