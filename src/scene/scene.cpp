#include "engine/scene/scene.hpp"

namespace arbor {
    namespace engine {
        scene::scene(const std::string& name, const std::filesystem::path& vertex_shader_src,
                     const std::filesystem::path& fragment_shader_src)
            : m_name(name), m_vertex_shader(vertex_shader_src), m_fragment_shader(fragment_shader_src) {}

        bool scene::operator==(const engine::scene& other) const {
            return std::hash<engine::scene>()(other) == std::hash<engine::scene>()(*this);
        }
    } // namespace engine
} // namespace arbor

std::size_t std::hash<arbor::engine::scene>::operator()(const arbor::engine::scene& scene) const {
    std::size_t hash =
        std::hash<std::filesystem::path>()(scene.m_vertex_shader) + std::hash<std::filesystem::path>()(scene.m_fragment_shader);

    // this is gonna miss a couple bytes at the end of the vector but we don't really care about that
    for (auto it = reinterpret_cast<const uint64_t*>(scene.m_objects.data());
         it < reinterpret_cast<const uint64_t*>(scene.m_objects.data() + scene.m_objects.size() - 1); it++) {
        hash += *it;
    }

    return hash;
}