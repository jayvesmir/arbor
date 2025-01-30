#include "arbor/scene/controls.hpp"

#include "imgui.h"

namespace arbor {
    namespace scene {
        namespace controls {
            std::expected<void, std::string> slider_f32::imgui_draw(const std::string& label) {
                ImGui::SliderFloat(label.c_str(), &m_value, m_start, m_end);
                return {};
            }
        } // namespace controls
    } // namespace scene
} // namespace arbor