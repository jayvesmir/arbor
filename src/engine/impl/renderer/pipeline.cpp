#include "engine/components/renderer.hpp"

namespace arbor {
    namespace engine {
        std::expected<void, std::string> renderer::make_vk_pipeline() {
            m_pipelines.emplace_back(vk.device);

            m_pipelines.back().bind_shader("src/shaders/basic.vert", shader::vertex);
            m_pipelines.back().bind_shader("src/shaders/basic.frag", shader::fragment);

            if (auto res = m_pipelines.front().reload(); !res)
                return res;

            return {};
        }

        renderer::pipeline::~pipeline() {}

        std::expected<void, std::string> renderer::pipeline::reload() {
            return std::unexpected("renderer::pipeline::reload(): unimplemented");
        }

        std::expected<void, std::string> renderer::pipeline::bind_shader(const std::filesystem::path& glsl_source, shader::etype type) {
            m_shaders[type] = {glsl_source, type, m_vk_device};

            m_logger->debug("binding glsl shader: {}", m_shaders[type].source().c_str());
            if (auto res = m_shaders[type].compile(); !res)
                return res;

            return {};
        }
    } // namespace engine
} // namespace arbor