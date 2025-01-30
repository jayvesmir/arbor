#pragma once

#include "arbor/components/components.hpp"

#include <expected>
#include <string>

namespace arbor {
    namespace scene {
        namespace controls {
            class control {
                friend class engine::renderer;

              public:
                control() = default;
                virtual ~control() = default;

              protected:
                virtual std::expected<void, std::string> imgui_draw(const std::string& label) { return {}; };
            };

            class slider_f32 : public controls::control {
                friend class engine::renderer;

                float32_t m_value;
                float32_t m_start, m_end;

              public:
                ~slider_f32() = default;
                slider_f32(float32_t start = 0.0f, float32_t end = 1.0f, float32_t value = 0.5f)
                    : m_value(value), m_start(start), m_end(end) {}

                constexpr auto value() const { return m_value; }

              protected:
                std::expected<void, std::string> imgui_draw(const std::string& label) override;
            };
        } // namespace controls
    } // namespace scene
} // namespace arbor