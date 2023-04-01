/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */


#ifndef SALMIAC_LOG_H
#define SALMIAC_LOG_H

#include "spdlog/spdlog.h"

namespace sal {

class Log {
public:
    static void init(std::string&& file_name) noexcept;

    template<typename... Args>
    static void info(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        spdlog::get(logger_name)->info(fmt, std::forward<Args>(args)...);
    }
    static void info(std::string const& message) noexcept;

    template<typename... Args>
    static void warn(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        spdlog::get(logger_name)->warn(fmt, std::forward<Args>(args)...);
    }
    static void warn(std::string const& message) noexcept;

    template<typename... Args>
    static void error(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        spdlog::get(logger_name)->error(fmt, std::forward<Args>(args)...);
    }
    static void error(std::string const& message) noexcept;

    template<typename... Args>
    static void fatal(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        spdlog::get(logger_name)->critical(fmt, std::forward<Args>(args)...);
    }
    static void fatal(std::string const& message) noexcept;

private:
    static std::string const logger_name;
};

} // namespace sal

#endif //SALMIAC_LOG_H
