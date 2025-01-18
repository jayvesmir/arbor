#pragma once

#include <unordered_map>

#include "glm/vec2.hpp"

#include "arbor/window.hpp"

namespace arbor {
    namespace engine {
        class input_manager {
          public:
            using key_state = uint32_t;

            enum key_state_bits : key_state {
                down = 1 << 1,
                held = 1 << 2,
            };

          private:
            glm::vec2 m_mouse_delta;
            glm::vec2 m_mouse_position;
            std::unordered_map<SDL_Scancode, key_state> m_keyboard_state;

          public:
            input_manager() {
                m_keyboard_state.reserve(SDL_SCANCODE_COUNT);

                // TODO: fill the maps with default values
            }

            void update_from_event(const SDL_Event& event);

            constexpr auto mouse_delta() const { return m_mouse_delta; }
            constexpr auto mouse_position() const { return m_mouse_position; }

            constexpr auto key_down(SDL_Scancode key) const {
                return static_cast<bool>(m_keyboard_state.at(key) & input_manager::down);
            }

            constexpr auto key_held(SDL_Scancode key) const {
                return static_cast<bool>(m_keyboard_state.at(key) & input_manager::held);
            }

            constexpr auto key_down(SDL_Scancode key) { return static_cast<bool>(m_keyboard_state[key] & input_manager::down); }
            constexpr auto key_held(SDL_Scancode key) { return static_cast<bool>(m_keyboard_state[key] & input_manager::held); }
        };
    } // namespace engine
} // namespace arbor