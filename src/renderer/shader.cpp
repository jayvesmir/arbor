#include "engine/components/renderer.hpp"
#include <cstring>
#include <fstream>
#include <iterator>

#include "shaderc/shaderc.h"
#include "shaderc/shaderc.hpp"
#include "shaderc/status.h"
#include "vulkan/vk_enum_string_helper.h"
#include "vulkan/vulkan_core.h"

namespace arbor {
    namespace engine {
        renderer::shader::~shader() {
            if (m_vk_shader && m_vk_device)
                vkDestroyShaderModule(m_vk_device, m_vk_shader, nullptr);
        }

        std::expected<void, std::string> renderer::shader::compile() {
            shaderc::Compiler compiler;
            shaderc::CompileOptions options;

            std::ifstream stream(m_source, std::ios::binary);
            if (!stream)
                return std::unexpected(fmt::format("failed to open shader source: {}", std::strerror(errno)));

            m_glsl = {std::istreambuf_iterator(stream), std::istreambuf_iterator<char>()};

            static std::unordered_map<shader::etype, shaderc_shader_kind> type_translation_map = {
                {etype::vertex, shaderc_vertex_shader},
                {etype::fragment, shaderc_fragment_shader},
            };

#ifdef NDEBUG
            options.SetOptimizationLevel(shaderc_optimization_level_performance);
#else
            options.SetOptimizationLevel(shaderc_optimization_level_zero);
#endif

            auto res = compiler.CompileGlslToSpv(m_glsl, type_translation_map[m_type], m_source.c_str());
            if (res.GetCompilationStatus() != shaderc_compilation_status_success)
                return std::unexpected(res.GetErrorMessage());

            m_spv = {res.begin(), res.end()};

            VkShaderModuleCreateInfo create_info{};

            create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            create_info.pCode    = m_spv.data();
            create_info.codeSize = m_spv.size() * sizeof(uint32_t);

            if (auto res = vkCreateShaderModule(m_vk_device, &create_info, nullptr, &m_vk_shader); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to create a shader module: {}", string_VkResult(res)));

            return {};
        }

        VkShaderStageFlagBits renderer::shader::stage() const {
            static std::unordered_map<shader::etype, VkShaderStageFlagBits> type_translation_map = {
                {etype::vertex, VK_SHADER_STAGE_VERTEX_BIT},
                {etype::fragment, VK_SHADER_STAGE_FRAGMENT_BIT},
            };
            return type_translation_map[m_type];
        }
    } // namespace engine
} // namespace arbor