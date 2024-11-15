#include "arbor/scene/scene.hpp"

namespace arbor {
    namespace engine {
        scene::scene(const std::string& name, const std::filesystem::path& vertex_shader_src,
                     const std::filesystem::path& fragment_shader_src)
            : m_name(name), m_vertex_shader(vertex_shader_src), m_fragment_shader(fragment_shader_src) {}
    } // namespace engine
} // namespace arbor