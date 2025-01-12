#pragma once
#include "arbor/scene/asset_library.hpp"
#include "arbor/scene/object.hpp"

#include <filesystem>
#include <string>
#include <unordered_map>

namespace arbor {
    namespace engine {
        class instance;

        class scene {
            friend class engine::instance;

            std::string m_name;

            std::filesystem::path m_vertex_shader;
            std::filesystem::path m_fragment_shader;

            engine::asset_library m_asset_library;
            std::unordered_map<uint64_t, engine::object> m_objects;

          public:
            scene(const std::string& name, const std::filesystem::path& vertex_shader_src = "",
                  const std::filesystem::path& fragment_shader_src = "");

            bool operator==(const engine::scene& other) const;

            constexpr auto vertex_shader(const std::filesystem::path& src) { m_vertex_shader = src; }
            constexpr auto fragment_shader(const std::filesystem::path& src) { m_fragment_shader = src; }

            std::expected<uint64_t, std::string> create_object();

            constexpr auto& vertex_shader() const { return m_vertex_shader; }
            constexpr auto& fragment_shader() const { return m_fragment_shader; }

            constexpr auto& name() const { return m_name; }
            constexpr auto& objects() { return m_objects; }
            constexpr auto& objects() const { return m_objects; }
            constexpr auto& asset_library() { return m_asset_library; }
            constexpr auto& asset_library() const { return m_asset_library; }
        };
    } // namespace engine
} // namespace arbor