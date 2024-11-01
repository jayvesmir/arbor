module;
#include <string>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

export module engine.logger_utils;

namespace arbor {
    namespace engine {
        export auto make_logger(const std::string& name) {
            auto logger = spdlog::stdout_color_mt(name);

            logger->disable_backtrace();
            logger->set_pattern("(%X.%e) {%n} [%^%l%$] %v");
#if NDEBUG
            logger->set_level(spdlog::level::info);
#else
            logger->set_level(spdlog::level::trace);
#endif
            return logger;
        }
    } // namespace engine
} // namespace arbor