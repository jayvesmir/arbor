#include "engine/components/renderer.hpp"
#include <ranges>

#include "vulkan/vk_enum_string_helper.h"

namespace arbor {
    namespace engine {
        std::expected<void, std::string> renderer::make_vk_pipeline() {
            m_pipelines.emplace_back(*this);

            m_pipelines.back().bind_shader("src/shaders/basic.vert", shader::vertex);
            m_pipelines.back().bind_shader("src/shaders/basic.frag", shader::fragment);

            if (auto res = m_pipelines.back().reload(); !res)
                return res;

            return {};
        }

        renderer::pipeline::~pipeline() {
            if (m_pipeline)
                vkDestroyPipeline(m_parent.vk.device, m_pipeline, nullptr);

            if (m_pipeline_layout)
                vkDestroyPipelineLayout(m_parent.vk.device, m_pipeline_layout, nullptr);

            if (m_render_pass)
                vkDestroyRenderPass(m_parent.vk.device, m_render_pass, nullptr);
        }

        std::expected<void, std::string> renderer::pipeline::reload() {
            m_parent.m_logger->debug("creating a vulkan pipeline layout");

            if (m_pipeline_layout)
                vkDestroyPipelineLayout(m_parent.vk.device, m_pipeline_layout, nullptr);

            if (m_render_pass)
                vkDestroyRenderPass(m_parent.vk.device, m_render_pass, nullptr);

            if (m_pipeline)
                vkDestroyPipeline(m_parent.vk.device, m_pipeline, nullptr);

            m_dynamic_state.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            m_dynamic_state.dynamicStateCount = m_dynamic_states.size();
            m_dynamic_state.pDynamicStates    = m_dynamic_states.data();

            m_vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

            m_input_assembly_state.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            m_input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

            m_rasterizer_state.sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            m_rasterizer_state.polygonMode = VK_POLYGON_MODE_FILL;
            m_rasterizer_state.lineWidth   = 1.0f;
            m_rasterizer_state.cullMode    = VK_CULL_MODE_BACK_BIT;
            m_rasterizer_state.frontFace   = VK_FRONT_FACE_CLOCKWISE;

            m_multisampler_state.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            m_multisampler_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            m_multisampler_state.minSampleShading     = 1.0f;

            m_color_blend_attachment.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

            m_color_blend_state.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            m_color_blend_state.attachmentCount = 1;
            m_color_blend_state.pAttachments    = &m_color_blend_attachment;

            m_pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

            if (auto res = vkCreatePipelineLayout(m_parent.vk.device, &m_pipeline_layout_create_info, nullptr, &m_pipeline_layout);
                res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to create a pipeline layout: {}", string_VkResult(res)));

            m_parent.m_logger->debug("creating a vulkan render pass");

            VkSubpassDependency subpass_dependency{};
            VkSubpassDescription subpass_description{};
            VkRenderPassCreateInfo render_pass_create_info{};
            VkAttachmentReference color_attachment_reference{};
            VkAttachmentDescription color_attachment_description{};

            subpass_dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
            subpass_dependency.dstSubpass    = 0;
            subpass_dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            subpass_dependency.srcAccessMask = 0;
            subpass_dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            color_attachment_description.format         = m_parent.vk.swapchain.format.format;
            color_attachment_description.samples        = VK_SAMPLE_COUNT_1_BIT;
            color_attachment_description.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            color_attachment_description.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            color_attachment_description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            color_attachment_description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            color_attachment_description.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            color_attachment_reference.attachment = 0;
            color_attachment_reference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            subpass_description.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass_description.colorAttachmentCount = 1;
            subpass_description.pColorAttachments    = &color_attachment_reference;

            render_pass_create_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            render_pass_create_info.attachmentCount = 1;
            render_pass_create_info.pAttachments    = &color_attachment_description;
            render_pass_create_info.subpassCount    = 1;
            render_pass_create_info.pSubpasses      = &subpass_description;
            render_pass_create_info.dependencyCount = 1;
            render_pass_create_info.pDependencies   = &subpass_dependency;

            if (auto res = vkCreateRenderPass(m_parent.vk.device, &render_pass_create_info, nullptr, &m_render_pass); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to create a vulkan render pass: {}", string_VkResult(res)));

            VkGraphicsPipelineCreateInfo pipeline_create_info{};

            m_parent.m_logger->debug("creating a vulkan pipeline with {} stages", m_pipeline_stages.size());

            m_viewport.maxDepth = 1.0f;
            m_viewport.width    = m_parent.vk.swapchain.extent.width;
            m_viewport.height   = m_parent.vk.swapchain.extent.height;

            m_scissor.extent = m_parent.vk.swapchain.extent;

            m_viewport_state.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            m_viewport_state.viewportCount = 1;
            m_viewport_state.pViewports    = &m_viewport;
            m_viewport_state.scissorCount  = 1;
            m_viewport_state.pScissors     = &m_scissor;

            pipeline_create_info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipeline_create_info.stageCount          = m_pipeline_stages.size();
            pipeline_create_info.pStages             = m_pipeline_stages.data();
            pipeline_create_info.pVertexInputState   = &m_vertex_input_state;
            pipeline_create_info.pInputAssemblyState = &m_input_assembly_state;
            pipeline_create_info.pViewportState      = &m_viewport_state;
            pipeline_create_info.pRasterizationState = &m_rasterizer_state;
            pipeline_create_info.pMultisampleState   = &m_multisampler_state;
            pipeline_create_info.pColorBlendState    = &m_color_blend_state;
            pipeline_create_info.pDynamicState       = &m_dynamic_state;

            pipeline_create_info.subpass    = 0;
            pipeline_create_info.renderPass = m_render_pass;
            pipeline_create_info.layout     = m_pipeline_layout;

            pipeline_create_info.basePipelineHandle = m_pipeline;

            if (auto res = vkCreateGraphicsPipelines(m_parent.vk.device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &m_pipeline);
                res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to create a vulkan pipeline: {}", string_VkResult(res)));

            return {};
        }

        std::expected<void, std::string> renderer::pipeline::bind_shader(const std::filesystem::path& glsl_source, shader::etype type) {
            if (m_shaders.contains(type))
                m_shaders.erase(type);

            m_shaders[type] = {glsl_source, type, m_parent.vk.device};

            m_parent.m_logger->debug("binding glsl shader: {}", m_shaders[type].source().c_str());
            if (auto res = m_shaders[type].compile(); !res)
                return res;

            VkPipelineShaderStageCreateInfo* create_info;

            if (auto it =
                    std::ranges::find_if(m_pipeline_stages, [&](const auto& stage) { return stage.stage == m_shaders[type].stage(); });
                it == m_pipeline_stages.end()) {
                m_pipeline_stages.push_back({});
                create_info = &m_pipeline_stages.back();
            } else {
                create_info = &*it;
            }

            create_info->sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            create_info->stage  = m_shaders[type].stage();
            create_info->module = m_shaders[type].shader_module();
            create_info->pName  = "main";

            return {};
        }
    } // namespace engine
} // namespace arbor