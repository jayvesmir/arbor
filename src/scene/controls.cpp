#include "arbor/scene/controls.hpp"

#include "imgui.h"

namespace arbor {
    namespace scene {
        namespace controls {
            std::expected<void, std::string> slider_f32::imgui_draw(engine::instance& engine, const std::string& label) {
                ImGui::SliderFloat(label.c_str(), &m_value, m_start, m_end);
                return {};
            }

            std::expected<void, std::string> button::imgui_draw(engine::instance& engine, const std::string& label) {
                if (ImGui::Button(label.c_str()))
                    std::invoke(m_callback, engine);
                return {};
            }
        } // namespace controls
    } // namespace scene
} // namespace arbor