module;
#include <expected>
#include <memory>
#include <string>

#include "spdlog/spdlog.h"

export module engine.components;
import arbor;
import engine.logger_utils;

namespace arbor {
    namespace engine {
        class instance;

        export class component {
          public:
            enum etype {
                invalid  = -1,
                renderer = 0,
            };

          protected:
            std::string m_identifier = "invalid";
            component::etype m_type  = etype::invalid;
            std::shared_ptr<spdlog::logger> m_logger;

          public:
            virtual ~component() = default;

            auto initialize_component_logger();
            constexpr auto type() const { return m_type; }
            constexpr auto identifier() const { return m_identifier; }

            virtual void shutdown()                         = 0;
            virtual std::expected<void, std::string> init() = 0;
        };

        auto component::initialize_component_logger() {
            m_logger = engine::make_logger(m_identifier);
        }
    } // namespace engine
} // namespace arbor