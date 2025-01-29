#include "arbor/scene/controls.hpp"

#include "imgui.h"

namespace arbor {
    namespace engine {
        namespace scene_controls {
            std::expected<void, std::string> slider_f32::imgui_draw(const std::string& label) {
                ImGui::SliderFloat(label.c_str(), &m_value, m_start, m_end);
                return {};
            }
        } // namespace scene_controls
    } // namespace engine
} // namespace arbor