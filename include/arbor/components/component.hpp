#pragma once
#include <expected>
#include <memory>
#include <string>

#include "spdlog/spdlog.h"

#include "arbor/logger_utils.hpp"
#include "arbor/types.hpp"

namespace arbor {
    namespace engine {
        class component {
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
            component()          = default;
            virtual ~component() = default;

            component(component&&)      = delete;
            component(const component&) = delete;

            std::shared_ptr<spdlog::logger> initialize_component_logger();
            constexpr auto type() const { return m_type; }
            constexpr auto identifier() const { return m_identifier; }

            virtual void shutdown()                           = 0;
            virtual std::expected<void, std::string> init()   = 0;
            virtual std::expected<void, std::string> update() = 0;
        };
    } // namespace engine
} // namespace arbor