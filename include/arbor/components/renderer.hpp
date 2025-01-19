#pragma once
#include <expected>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "arbor/components/component.hpp"
#include "arbor/engine.hpp"
#include "arbor/model.hpp"
#include "arbor/scene/texture.hpp"
#include "arbor/types.hpp"
#include "arbor/window.hpp"

#include "imgui.h"
#include "imgui_internal.h"
#include "vulkan/vulkan.h"

namespace arbor {
    namespace engine {
        namespace detail {
            struct device_queue_family_indices {
                uint32_t graphics_family;
                uint32_t present_family;
            };

            struct mvp {
                glm::mat4 model;
                glm::mat4 view;
                glm::mat4 projection;
            };
        } // namespace detail

        class renderer : public engine::component {
          public:
            class shader {
                friend class renderer;

              public:
                enum etype {
                    invalid = -1,
                    vertex = 0,
                    fragment = 1,
                };

              private:
                std::filesystem::path m_source;
                shader::etype m_type = etype::invalid;

                std::string m_glsl;
                std::vector<uint32_t> m_spv;

                VkDevice m_vk_device = VK_NULL_HANDLE;
                VkShaderModule m_vk_shader = VK_NULL_HANDLE;

              public:
                ~shader();
                shader() = default;
                shader(const std::filesystem::path& glsl_source, shader::etype type, VkDevice device)
                    : m_source(glsl_source), m_type(type), m_vk_device(device) {}

                shader(shader&& other)
                    : m_source(std::move(other.m_source)), m_type(other.m_type), m_vk_device(other.m_vk_device),
                      m_vk_shader(other.m_vk_shader) {
                    other.m_type = etype::invalid;
                    other.m_vk_device = VK_NULL_HANDLE;
                    other.m_vk_shader = VK_NULL_HANDLE;
                }

                shader(const shader&) = delete;

                auto& operator=(shader&& other) {
                    m_source.swap(other.m_source);
                    m_type = other.m_type;
                    m_vk_device = other.m_vk_device;

                    other.m_source.clear();
                    other.m_type = etype::invalid;
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
                engine::renderer& m_renderer;

                VkPipelineLayoutCreateInfo m_pipeline_layout_create_info{};
                VkPipelineColorBlendAttachmentState m_color_blend_attachment{};
                VkPipelineDynamicStateCreateInfo m_dynamic_state{};
                VkPipelineViewportStateCreateInfo m_viewport_state{};
                VkPipelineRasterizationStateCreateInfo m_rasterizer_state{};
                VkPipelineVertexInputStateCreateInfo m_vertex_input_state{};
                VkPipelineMultisampleStateCreateInfo m_multisampler_state{};
                VkPipelineColorBlendStateCreateInfo m_color_blend_state{};
                VkPipelineInputAssemblyStateCreateInfo m_input_assembly_state{};

                VkRect2D m_scissor{};
                VkViewport m_viewport{};

                VkPipeline m_pipeline = VK_NULL_HANDLE;
                VkRenderPass m_render_pass = VK_NULL_HANDLE;
                VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;

                std::vector<VkDescriptorSet> m_descriptor_sets;
                VkDescriptorPool m_descriptor_pool = VK_NULL_HANDLE;
                VkDescriptorSetLayout m_descriptor_set_layout = VK_NULL_HANDLE;

                std::vector<VkDynamicState> m_dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
                std::vector<VkPipelineShaderStageCreateInfo> m_pipeline_stages;

                std::unordered_map<shader::etype, engine::renderer::shader> m_shaders;

              public:
                ~pipeline();
                pipeline(engine::renderer& parent) : m_renderer(parent) {}
                pipeline(pipeline&& other) : m_renderer(other.m_renderer), m_shaders(std::move(other.m_shaders)) {}

                pipeline(const pipeline&) = delete;

                std::expected<void, std::string> reload();
                std::expected<void, std::string> bind_shader(const std::filesystem::path& glsl_source, shader::etype type);

              protected:
                constexpr auto scissors() const { return &m_scissor; }
                constexpr auto viewports() const { return &m_viewport; }
                constexpr auto render_pass() const { return m_render_pass; }
                constexpr auto descriptor_pool() const { return m_descriptor_pool; }
                constexpr auto pipeline_handle() const { return m_pipeline; }

              private:
                std::expected<void, std::string> make_vk_descriptor_pool_and_sets();
            };

            class device_buffer {
                VkDevice m_device = VK_NULL_HANDLE;
                VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;

                VkBuffer m_buffer = VK_NULL_HANDLE;
                VkDeviceMemory m_memory = VK_NULL_HANDLE;

                uint64_t m_size = 0;
                bool m_keep_mapped = false;
                void* m_mapped = nullptr;

              public:
                ~device_buffer();
                std::expected<void, std::string> make(uint64_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                                                      VkDevice device, VkPhysicalDevice physical_device,
                                                      bool keep_mapped = false);

                std::expected<void, std::string> write_data(const void* bytes, uint64_t size,
                                                            VkQueue transfer_queue = VK_NULL_HANDLE,
                                                            VkCommandPool command_pool = VK_NULL_HANDLE);

                void free();

                constexpr auto size() const { return m_size; }
                constexpr auto buffer() { return &m_buffer; }
                constexpr auto buffer() const { return &m_buffer; }
                constexpr auto memory() { return &m_memory; }
                constexpr auto memory() const { return &m_memory; }
            };

            class texture {
                VkDevice m_device;
                VkPhysicalDevice m_physical_device;

                renderer::device_buffer m_staging_buffer;
                uint32_t m_width = 0, m_height = 0;

                VkImage m_image = VK_NULL_HANDLE;
                VkImageView m_image_view = VK_NULL_HANDLE;
                VkDeviceMemory m_image_memory = VK_NULL_HANDLE;
                VkSampler m_sampler = VK_NULL_HANDLE;

                std::expected<void, std::string> transition_layout(engine::renderer& renderer, VkImageLayout old_layout,
                                                                   VkImageLayout new_layout);

              public:
                ~texture();
                texture() = default;
                texture(VkDevice device, VkPhysicalDevice physical_device)
                    : m_device(device), m_physical_device(physical_device) {}

                void destroy();

                std::expected<void, std::string> load(const engine::texture& source, engine::renderer& renderer);

                constexpr auto image() { return m_image; }
                constexpr auto sampler() { return m_sampler; }
                constexpr auto image_view() { return m_image_view; }
            };

          private:
            engine::instance& m_engine;

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
                    VkSurfaceKHR surface = VK_NULL_HANDLE;

                    uint32_t current_image;
                    std::vector<VkImage> images;
                    std::vector<VkImageView> image_views;
                    std::vector<VkFramebuffer> framebuffers;

                    VkExtent2D extent;
                    VkSurfaceFormatKHR format;
                    VkPresentModeKHR present_mode;
                    VkSurfaceCapabilitiesKHR surface_capabilities;
                } swapchain;

                VkDevice device = VK_NULL_HANDLE;
                VkQueue graphics_queue = VK_NULL_HANDLE;
                VkQueue present_queue = VK_NULL_HANDLE;

                VkCommandPool command_pool = VK_NULL_HANDLE;
                std::vector<VkCommandBuffer> command_buffers;
                std::vector<VkCommandBuffer> temporary_command_buffers;

                renderer::device_buffer index_buffer;
                renderer::device_buffer vertex_buffer;
                std::vector<renderer::device_buffer> uniform_buffers;

                struct {
                    const uint32_t frames_in_flight = 3;

                    uint32_t current_frame = 0;
                    std::vector<VkSemaphore> wait_semaphores;
                    std::vector<VkSemaphore> signal_semaphores;
                    std::vector<VkFence> in_flight_fences;

                    VkPresentInfoKHR present_info{};
                } sync;

                struct {
                    VkPresentModeKHR present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
                } config;

                std::atomic<bool> deferred_swapchain_reload = false;

                std::vector<const char*> device_ext;
                std::vector<const char*> device_lay;
            } vk;

            std::vector<renderer::pipeline> m_pipelines;

            std::unordered_map<uint64_t, std::unordered_map<engine::texture::etype, renderer::texture>> m_textures;

            struct {
                ImGuiContext* imgui_ctx = nullptr;
            } m_gui;

          public:
            renderer(engine::instance& parent);

            renderer(renderer&&) = delete;
            renderer(const renderer&) = delete;

            void shutdown() override;
            std::expected<void, std::string> init() override;
            std::expected<void, std::string> update() override;

            std::expected<void, std::string> resize_viewport();

          private:
            std::expected<uint32_t, std::string> acquire_image();
            std::expected<void, std::string> reload_swapchain();
            std::expected<void, std::string> update_ubos();
            std::expected<void, std::string> draw_gui();

            std::expected<void, std::string> record_command_buffer();
            std::expected<void, std::string> submit_and_present_current_command_buffer();

            std::expected<void, std::string> init_imgui();
            std::expected<void, std::string> load_assets();

            std::expected<void, std::string> make_vk_instance();
            std::expected<void, std::string> make_vk_device();
            std::expected<void, std::string> make_vk_surface();
            std::expected<void, std::string> make_vk_pipeline();
            std::expected<void, std::string> make_vk_swapchain_and_pipeline();
            std::expected<void, std::string> make_vk_command_pool_and_buffers();
            std::expected<void, std::string> make_sync_objects();

            std::expected<void, std::string> make_vertex_buffer();
            std::expected<void, std::string> make_index_buffer();
            std::expected<void, std::string> make_uniform_buffers();

            std::expected<VkCommandBuffer, std::string> begin_temporary_command_buffer();
            std::expected<void, std::string> submit_command_buffer(VkCommandBuffer command_buffer, VkQueue queue);
        };
    } // namespace engine
} // namespace arbor