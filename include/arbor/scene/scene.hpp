#pragma once
#include "arbor/assets/library.hpp"
#include "arbor/components/components.hpp"
#include "arbor/configs.hpp"
#include "arbor/scene/camera.hpp"
#include "arbor/scene/controls.hpp"
#include "arbor/scene/object.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace arbor {
    namespace scene {
        class instance {
            friend class engine::instance;
            friend class engine::renderer;

            std::string m_name;

            std::filesystem::path m_vertex_shader;
            std::filesystem::path m_fragment_shader;

            engine::camera m_camera;
            assets::library m_asset_library;

            engine::internal_callback_config m_internal_callbacks;

            std::vector<uint64_t> m_drawable_objects;
            std::unordered_map<uint64_t, engine::object> m_objects;
            std::unordered_map<std::string, std::shared_ptr<scene::controls::control>> m_controls;

          public:
            instance(const std::string& name, const std::filesystem::path& vertex_shader_src = "",
                     const std::filesystem::path& fragment_shader_src = "");

            bool operator==(const scene::instance& other) const;

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

            std::expected<void, std::string> commit();
            bool is_object_drawable(uint64_t object_id);

            template <typename T> constexpr void add_control(const std::string& label, const auto&... ctor_args);
            template <typename T>
            constexpr std::expected<std::shared_ptr<T>, std::string> control(const std::string& label) const {
                if (!m_controls.contains(label))
                    return std::unexpected(fmt::format("no control named '{}'", label));
                return std::dynamic_pointer_cast<T>(m_controls.at(label));
            }

          protected:
            constexpr auto& controls() { return m_controls; }
            constexpr auto& drawable_objects() { return m_drawable_objects; }

            constexpr auto& internal_callbacks() const { return m_internal_callbacks; }
            constexpr auto internal_callbacks(const engine::internal_callback_config& config) { m_internal_callbacks = config; }
        };

        template <typename T> constexpr void instance::add_control(const std::string& label, const auto&... ctor_args) {
            m_controls.emplace(label, std::make_shared<T>(ctor_args...));
        }
    } // namespace scene
} // namespace arbor