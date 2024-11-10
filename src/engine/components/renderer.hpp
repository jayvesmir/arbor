#pragma once
#include <expected>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "arbor.hpp"
#include "engine/components/component.hpp"
#include "engine/engine.hpp"
#include "engine/logger_utils.hpp"
#include "engine/window.hpp"

#include "vulkan/vulkan.h"

namespace arbor {
    namespace engine {
        namespace detail {
            struct device_queue_family_indices {
                uint32_t graphics_family;
                uint32_t present_family;
            };
        } // namespace detail

        class renderer : public engine::component {
          public:
            class shader {
                friend class renderer;

              public:
                enum etype {
                    invalid  = -1,
                    vertex   = 0,
                    fragment = 1,
                };

              private:
                std::filesystem::path m_source;
                shader::etype m_type = etype::invalid;

                std::string m_glsl;
                std::vector<uint32_t> m_spv;

                VkDevice m_vk_device       = VK_NULL_HANDLE;
                VkShaderModule m_vk_shader = VK_NULL_HANDLE;

              public:
                ~shader();
                shader() = default;
                shader(const std::filesystem::path& glsl_source, shader::etype type, VkDevice device)
                    : m_source(glsl_source), m_type(type), m_vk_device(device) {}

                shader(shader&& other)
                    : m_source(std::move(other.m_source)), m_type(other.m_type), m_vk_device(other.m_vk_device),
                      m_vk_shader(other.m_vk_shader) {
                    other.m_type      = etype::invalid;
                    other.m_vk_device = VK_NULL_HANDLE;
                    other.m_vk_shader = VK_NULL_HANDLE;
                }

                shader(const shader&) = delete;

                auto& operator=(shader&& other) {
                    m_source.swap(other.m_source);
                    m_type      = other.m_type;
                    m_vk_device = other.m_vk_device;

                    other.m_source.clear();
                    other.m_type      = etype::invalid;
                    other.m_vk_device = VK_NULL_HANDLE;
                    other.m_vk_shader = VK_NULL_HANDLE;

                    return *this;
                }

                std::expected<void, std::string> compile();

                VkShaderStageFlagBits stage() const;
                auto source() const { return m_source; }
                constexpr auto type() const { return m_type; }
                constexpr auto shader_module() const { return m_vk_shader; }

              protected:
                constexpr auto glsl() const { return m_glsl; }
                constexpr auto spv() const { return m_spv; }
            };

            class pipeline {
                friend class renderer;
                const engine::renderer& m_parent;
                bool m_initialized = false;

                VkPipelineLayoutCreateInfo m_pipeline_layout_create_info{};
                VkPipelineColorBlendAttachmentState m_color_blend_attachment{};
                VkPipelineDynamicStateCreateInfo m_dynamic_state{};
                VkPipelineViewportStateCreateInfo m_viewport_state{};
                VkPipelineRasterizationStateCreateInfo m_rasterizer_state{};
                VkPipelineVertexInputStateCreateInfo m_vertex_input_state{};
                VkPipelineMultisampleStateCreateInfo m_multisampler_state{};
                VkPipelineColorBlendStateCreateInfo m_color_blend_state{};
                VkPipelineInputAssemblyStateCreateInfo m_input_assembly_state{};

                VkRect2D m_scissor;
                VkViewport m_viewport;

                VkPipeline m_pipeline;
                VkRenderPass m_render_pass;
                VkPipelineLayout m_pipeline_layout;

                std::vector<VkDynamicState> m_dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
                std::vector<VkPipelineShaderStageCreateInfo> m_changed_pipeline_stages;

                std::unordered_map<shader::etype, engine::renderer::shader> m_shaders;

              public:
                ~pipeline();
                pipeline(const engine::renderer& parent) : m_parent(parent) {}
                pipeline(pipeline&& other) : m_parent(other.m_parent), m_shaders(std::move(other.m_shaders)) {}

                pipeline(const pipeline&) = delete;

                std::expected<void, std::string> reload();
                std::expected<void, std::string> bind_shader(const std::filesystem::path& glsl_source, shader::etype type);
            };

          private:
            const engine::instance& m_parent;

            struct {
                VkInstance instance = VK_NULL_HANDLE;
                std::vector<const char*> instance_ext;
                std::vector<const char*> instance_lay;

                VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;

                struct {
                    VkPhysicalDevice handle = VK_NULL_HANDLE;
                    VkPhysicalDeviceFeatures features;
                    VkPhysicalDeviceProperties properties;

                    detail::device_queue_family_indices queue_family_indices;
                } physical_device;

                struct {
                    VkSwapchainKHR handle = VK_NULL_HANDLE;
                    VkSurfaceKHR surface  = VK_NULL_HANDLE;

                    std::vector<VkImage> images;
                    std::vector<VkImageView> image_views;

                    VkExtent2D extent;
                    VkSurfaceFormatKHR format;
                    VkPresentModeKHR present_mode;
                    VkSurfaceCapabilitiesKHR surface_capabilities;
                } swapchain;

                VkDevice device        = VK_NULL_HANDLE;
                VkQueue graphics_queue = VK_NULL_HANDLE;
                VkQueue present_queue  = VK_NULL_HANDLE;

                std::vector<const char*> device_ext;
                std::vector<const char*> device_lay;
            } vk;

            std::vector<renderer::pipeline> m_pipelines;

          public:
            renderer(const engine::instance& parent);
            ~renderer() = default;

            renderer(renderer&&)      = delete;
            renderer(const renderer&) = delete;

            void shutdown() override;
            std::expected<void, std::string> init() override;

          private:
            std::expected<void, std::string> make_vk_instance();
            std::expected<void, std::string> make_vk_device();
            std::expected<void, std::string> make_vk_surface();
            std::expected<void, std::string> make_vk_pipeline();
            std::expected<void, std::string> make_vk_swapchain();
        };
    } // namespace engine
} // namespace arbor