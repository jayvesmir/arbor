#include "arbor/input_manager.hpp"

#include "imgui.h"

namespace arbor {
    namespace engine {
        void input_manager::update_from_event(const SDL_Event& event) {
            switch (event.type) {
            case SDL_EVENT_KEY_DOWN: {
                m_keyboard_state[event.key.scancode] |= input_manager::down;
                if (event.key.repeat)
                    m_keyboard_state[event.key.scancode] |= input_manager::held;
            } break;

            case SDL_EVENT_KEY_UP: {
                m_keyboard_state[event.key.scancode] = 0;
            } break;

            case SDL_EVENT_MOUSE_MOTION: {
                auto last_pos = m_mouse_position;

                m_mouse_position.x = event.motion.x;
                m_mouse_position.y = event.motion.y;

                // TODO: Don't steal the handling from imgui
                m_mouse_delta.x = ImGui::GetIO().MouseDelta.x;
                m_mouse_delta.y = ImGui::GetIO().MouseDelta.y;
            } break;
            }
        }
    } // namespace engine
} // namespace arbor