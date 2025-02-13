#include "arbor/scene/scene.hpp"

#include <random>

namespace arbor {
    namespace scene {
        instance::instance(const std::string& name, const std::filesystem::path& vertex_shader_src,
                           const std::filesystem::path& fragment_shader_src)
            : m_name(name), m_vertex_shader(vertex_shader_src), m_fragment_shader(fragment_shader_src) {}

        std::expected<uint64_t, std::string> instance::create_object() {
            static std::mt19937_64 rng;
            static std::uniform_int_distribution<uint64_t> rng_dist(0);
            static std::random_device rng_device;

            uint64_t id = rng_dist(rng);
            while (m_objects.contains(id))
                id = rng_dist(rng);

            m_objects[id] = {id};
            m_asset_library[id] = {{}, assets::material::make_default()};

            return id;
        }

        bool instance::is_object_drawable(uint64_t object_id) {
            if (!m_asset_library.entries().contains(object_id))
                return false;

            if (m_asset_library[object_id].model.vertices.empty() || m_asset_library[object_id].model.indices.empty())
                return false;

            return true;
        }

        std::expected<void, std::string> instance::commit() {
            m_drawable_objects.clear();

            for (auto& [id, obj] : m_objects) {
                if (is_object_drawable(id))
                    m_drawable_objects.push_back(id);
            }

            if (m_internal_callbacks.on_scene_change.has_value()) {
                if (auto res = std::invoke(*m_internal_callbacks.on_scene_change); !res)
                    return res;
            }

            return {};
        }
    } // namespace scene
} // namespace arbor