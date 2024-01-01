/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "log.h"

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace sal {

std::string const Log::logger_name{"CH4Cl"};

void Log::init(std::string&& file_name) noexcept
{
    std::vector<spdlog::sink_ptr> const sinks{
        std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(std::move(file_name))};

    auto const combined_logger =
        std::make_shared<spdlog::logger>(logger_name, begin(sinks), end(sinks));

    // Register the logger, accessible globally with the name
    spdlog::register_logger(combined_logger);
}

void Log::debug(const std::string& message) noexcept
{
    spdlog::get(logger_name)->debug(message);
}

void Log::info(const std::string& message) noexcept
{
    spdlog::get(logger_name)->info(message);
}

void Log::warn(const std::string& message) noexcept
{
    spdlog::get(logger_name)->warn(message);
}

void Log::error(const std::string& message) noexcept
{
    spdlog::get(logger_name)->error(message);
}

void Log::fatal(const std::string& message) noexcept
{
    spdlog::get(logger_name)->critical(message);
}

} // namespace sal
