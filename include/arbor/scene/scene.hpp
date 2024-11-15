#pragma once
#include "arbor/scene/object.hpp"

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace arbor {
    namespace engine {
        class instance;

        class scene {
            friend class engine::instance;
            friend class std::hash<engine::scene>;

            std::string m_name;

            std::filesystem::path m_vertex_shader;
            std::filesystem::path m_fragment_shader;

            std::vector<engine::object> m_objects;

          public:
            scene(const std::string& name, const std::filesystem::path& vertex_shader_src = "",
                  const std::filesystem::path& fragment_shader_src = "");

            bool operator==(const engine::scene& other) const;

            constexpr auto vertex_shader(const std::filesystem::path& src) { m_vertex_shader = src; }
            constexpr auto fragment_shader(const std::filesystem::path& src) { m_fragment_shader = src; }

            constexpr auto& vertex_shader() const { return m_vertex_shader; }
            constexpr auto& fragment_shader() const { return m_fragment_shader; }

            constexpr auto pop_object() { m_objects.pop_back(); }
            constexpr auto push_object(const engine::object& obj) { m_objects.push_back(obj); }
            constexpr auto erase_object(const std::vector<engine::object>::const_iterator it) { m_objects.erase(it); }

            constexpr auto& name() const { return m_name; }
            constexpr auto& objects() const { return m_objects; }
        };
    } // namespace engine
} // namespace arbor