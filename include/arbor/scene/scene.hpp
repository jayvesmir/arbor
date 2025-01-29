#pragma once
#include "arbor/components/components.hpp"
#include "arbor/scene/asset_library.hpp"
#include "arbor/scene/camera.hpp"
#include "arbor/scene/controls.hpp"
#include "arbor/scene/object.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace arbor {
    namespace engine {
        class instance;

        class scene {
            friend class engine::instance;
            friend class engine::renderer;

            std::string m_name;

            std::filesystem::path m_vertex_shader;
            std::filesystem::path m_fragment_shader;

            engine::camera m_camera;
            engine::asset_library m_asset_library;
            std::unordered_map<uint64_t, engine::object> m_objects;
            std::unordered_map<std::string, std::shared_ptr<scene_controls::control>> m_controls;

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
            constexpr auto& camera() { return m_camera; }
            constexpr auto& camera() const { return m_camera; }
            constexpr auto& objects() { return m_objects; }
            constexpr auto& objects() const { return m_objects; }
            constexpr auto& asset_library() { return m_asset_library; }
            constexpr auto& asset_library() const { return m_asset_library; }
            constexpr auto& controls() const { return m_controls; }

            template <typename T> constexpr void add_control(const std::string& label, const auto&... ctor_args);
            template <typename T>
            constexpr std::expected<std::shared_ptr<T>, std::string> control(const std::string& label) const {
                if (!m_controls.contains(label))
                    return std::unexpected(fmt::format("no control named '{}'", label));
                return std::dynamic_pointer_cast<T>(m_controls.at(label));
            }

          protected:
            constexpr auto& controls() { return m_controls; }
        };

        template <typename T> constexpr void scene::add_control(const std::string& label, const auto&... ctor_args) {
            m_controls.emplace(label, std::make_shared<T>(ctor_args...));
        }
    } // namespace engine
} // namespace arbor