#include "arbor/logger_utils.hpp"

namespace arbor {
    std::shared_ptr<spdlog::logger> make_logger(const std::string& name) {
        auto logger = spdlog::stdout_color_mt(name);

        logger->disable_backtrace();
        logger->set_pattern("(%X.%e) {%n} [%^%l%$] %v");
#ifdef NDEBUG
        logger->set_level(spdlog::level::info);
#else
        logger->set_level(spdlog::level::trace);
#endif
        return logger;
    }
} // namespace arbor