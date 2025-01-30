#pragma once
#include <string>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace arbor {
    std::shared_ptr<spdlog::logger> make_logger(const std::string& name);
} // namespace arbor