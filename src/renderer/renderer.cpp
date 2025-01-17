#include "arbor/components/renderer.hpp"

#include "arbor/model.hpp"
#include "arbor/types.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/trigonometric.hpp"
#include "vulkan/vk_enum_string_helper.h"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include <algorithm>

namespace arbor {
    namespace engine {
        renderer::renderer(engine::instance& parent) : m_parent(parent) {
            m_identifier = "renderer";
            m_type = etype::renderer;
        }

        void renderer::shutdown() {
            if (vk.device)
                vkDeviceWaitIdle(vk.device);

            vk.index_buffer.free();
            vk.vertex_buffer.free();
            vk.uniform_buffers.clear();

            m_pipelines.clear();
            m_textures.clear();

            for (auto i = 0ull; i < vk.sync.frames_in_flight; i++) {
                if (vk.sync.signal_semaphores[i] && vk.device) {
                    vkDestroySemaphore(vk.device, vk.sync.signal_semaphores[i], nullptr);
                    vk.sync.signal_semaphores[i] = VK_NULL_HANDLE;
                }

                if (vk.sync.wait_semaphores[i] && vk.device) {
                    vkDestroySemaphore(vk.device, vk.sync.wait_semaphores[i], nullptr);
                    vk.sync.wait_semaphores[i] = VK_NULL_HANDLE;
                }

                if (vk.sync.in_flight_fences[i] && vk.device) {
                    vkDestroyFence(vk.device, vk.sync.in_flight_fences[i], nullptr);
                    vk.sync.in_flight_fences[i] = VK_NULL_HANDLE;
                }
            }

            if (vk.command_pool && vk.device) {
                vkDestroyCommandPool(vk.device, vk.command_pool, nullptr);
                vk.command_pool = VK_NULL_HANDLE;
            }

            for (auto& framebuffer : vk.swapchain.framebuffers) {
                if (framebuffer && vk.device)
                    vkDestroyFramebuffer(vk.device, framebuffer, nullptr);
                framebuffer = VK_NULL_HANDLE;
            }

            for (auto& image_view : vk.swapchain.image_views) {
                if (image_view && vk.device)
                    vkDestroyImageView(vk.device, image_view, nullptr);
                image_view = VK_NULL_HANDLE;
            }

            if (vk.swapchain.handle && vk.device) {
                vkDestroySwapchainKHR(vk.device, vk.swapchain.handle, nullptr);
                vk.swapchain.handle = VK_NULL_HANDLE;
            }

            if (vk.swapchain.surface && vk.instance) {
                vkDestroySurfaceKHR(vk.instance, vk.swapchain.surface, nullptr);
                vk.swapchain.surface = VK_NULL_HANDLE;
            }

            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplSDL3_Shutdown();
            ImGui::DestroyContext(m_gui.imgui_ctx);

            if (vk.device) {
                vkDestroyDevice(vk.device, nullptr);
                vk.device = VK_NULL_HANDLE;
            }

            if (vk.debug_messenger && vk.instance) {
                auto destroy_fn = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                    vkGetInstanceProcAddr(vk.instance, "vkDestroyDebugUtilsMessengerEXT"));
                destroy_fn(vk.instance, vk.debug_messenger, nullptr);
                vk.debug_messenger = VK_NULL_HANDLE;
            }

            if (vk.instance) {
                vkDestroyInstance(vk.instance, nullptr);
                vk.instance = VK_NULL_HANDLE;
            }
        }

        std::expected<void, std::string> renderer::init() {
            initialize_component_logger();
            m_logger->trace("parent: {}", fmt::ptr(&m_parent));
            m_logger->trace("parent window: {}", fmt::ptr(&m_parent.window()));

            if (auto res = make_vk_instance(); !res)
                return res;

            if (auto res = make_vk_surface(); !res)
                return res;

            if (auto res = make_vk_device(); !res)
                return res;

            if (auto res = make_vertex_buffer(); !res)
                return res;

            if (auto res = make_index_buffer(); !res)
                return res;

            if (auto res = make_uniform_buffers(); !res)
                return res;

            if (auto res = make_vk_command_pool_and_buffers(); !res)
                return res;

            if (auto res = load_assets(); !res)
                return res;

            if (auto res = make_vk_swapchain_and_pipeline(); !res)
                return res;

            if (auto res = make_sync_objects(); !res)
                return res;

            if (auto res = init_imgui(); !res)
                return res;

            return {};
        }

        std::expected<void, std::string> renderer::update() {
            if (vk.deferred_swapchain_reload) {
                reload_swapchain();
                vk.deferred_swapchain_reload = false;
            }

            vkWaitForFences(vk.device, 1, &vk.sync.in_flight_fences[vk.sync.current_frame], VK_TRUE, uint64_t(-1));

            auto image_idx = acquire_image();
            if (!image_idx)
                return std::unexpected(image_idx.error());

            if (*image_idx == uint32_t(-1))
                return {};

            vk.swapchain.current_image = *image_idx;

            vkResetFences(vk.device, 1, &vk.sync.in_flight_fences[vk.sync.current_frame]);
            vkResetCommandBuffer(vk.command_buffers[vk.sync.current_frame], 0);

            if (auto res = record_command_buffer(); !res)
                return res;

            if (auto res = submit_and_present_current_command_buffer(); !res)
                return res;

            vk.sync.current_frame++;
            vk.sync.current_frame %= vk.sync.frames_in_flight;

            return {};
        }

        std::expected<void, std::string> renderer::record_command_buffer() {
            static VkClearValue background{
                .color =
                    {
                        .float32 = {0.0f, 0.0f, 0.0f, 1.0f},
                    },
            };

            static VkCommandBufferBeginInfo cmd_buffer_begin_info{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            };

            const auto& current_cmd_buf = vk.command_buffers[vk.sync.current_frame];

            VkRenderPassBeginInfo render_pass_begin_info{};

            if (auto res = vkBeginCommandBuffer(current_cmd_buf, &cmd_buffer_begin_info); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to begin recording a command buffer: {}", string_VkResult(res)));

            render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            render_pass_begin_info.renderPass = m_pipelines.back().render_pass();
            render_pass_begin_info.framebuffer = vk.swapchain.framebuffers[vk.swapchain.current_image];
            render_pass_begin_info.renderArea.extent = vk.swapchain.extent;
            render_pass_begin_info.clearValueCount = 1;
            render_pass_begin_info.pClearValues = &background;

            vkCmdBeginRenderPass(current_cmd_buf, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(current_cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelines.back().pipeline_handle());

            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(current_cmd_buf, 0, 1, vk.vertex_buffer.buffer(), &offset);
            vkCmdBindIndexBuffer(current_cmd_buf, *vk.index_buffer.buffer(), 0, VK_INDEX_TYPE_UINT32);

            vkCmdSetViewport(current_cmd_buf, 0, 1, m_pipelines.back().viewports());
            vkCmdSetScissor(current_cmd_buf, 0, 1, m_pipelines.back().scissors());

            update_ubos();

            uint32_t index_offset = 0;
            uint32_t vertex_offset = 0;
            uint32_t ubo_offset = 0;
            for (auto& [id, object] : m_parent.current_scene().objects()) {

                vkCmdBindDescriptorSets(current_cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelines.back().m_pipeline_layout, 0,
                                        1, &m_pipelines.back().m_descriptor_sets[vk.sync.current_frame + ubo_offset], 0, nullptr);
                vkCmdDrawIndexed(current_cmd_buf, m_parent.current_scene().asset_library()[id].model.indices.size(), 1,
                                 index_offset, vertex_offset, 0);

                index_offset += m_parent.current_scene().asset_library()[id].model.indices.size();
                vertex_offset += m_parent.current_scene().asset_library()[id].model.vertices.size();
                ubo_offset += vk.sync.frames_in_flight;
            }

            draw_gui();

            return {};
        }

        std::expected<void, std::string> renderer::submit_and_present_current_command_buffer() {
            vkCmdEndRenderPass(vk.command_buffers[vk.sync.current_frame]);

            if (auto res = vkEndCommandBuffer(vk.command_buffers[vk.sync.current_frame]); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to record a command buffer: {}", string_VkResult(res)));

            VkSubmitInfo submit_info{};
            VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.waitSemaphoreCount = 1;
            submit_info.pWaitSemaphores = &vk.sync.wait_semaphores[vk.sync.current_frame];
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &vk.command_buffers[vk.sync.current_frame];
            submit_info.signalSemaphoreCount = 1;
            submit_info.pSignalSemaphores = &vk.sync.signal_semaphores[vk.sync.current_frame];
            submit_info.pWaitDstStageMask = wait_stages;

            if (auto res = vkQueueSubmit(vk.graphics_queue, 1, &submit_info, vk.sync.in_flight_fences[vk.sync.current_frame]);
                res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to submit the draw queue: {}", string_VkResult(res)));

            vk.sync.present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            vk.sync.present_info.waitSemaphoreCount = 1;
            vk.sync.present_info.pWaitSemaphores = &vk.sync.signal_semaphores[vk.sync.current_frame];
            vk.sync.present_info.swapchainCount = 1;
            vk.sync.present_info.pSwapchains = &vk.swapchain.handle;
            vk.sync.present_info.pImageIndices = &vk.swapchain.current_image;

            if (auto res = vkQueuePresentKHR(vk.present_queue, &vk.sync.present_info); res != VK_SUCCESS) {
                if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
                    resize_viewport();
                else
                    return std::unexpected(fmt::format("failed to present: {}", string_VkResult(res)));
            }

            return {};
        }

        std::expected<void, std::string> renderer::resize_viewport() {
            auto old_width = m_parent.window().width();
            auto old_height = m_parent.window().height();
            m_parent.window().update_dimensions();

            if (old_width == m_parent.window().width() && old_height == m_parent.window().height())
                return {};

            return reload_swapchain();
        }

        std::expected<void, std::string> renderer::update_ubos() {
            static engine::detail::mvp mvp;
            mvp.view = m_parent.current_scene().camera().view_matrix();

            mvp.projection = glm::perspective(
                glm::radians(75.0f), static_cast<float>(m_parent.window().width()) / m_parent.window().height(), 1e-6f, 1e+6f);
            mvp.projection[1][1] *= -1.0;

            uint32_t ubo_offset = 0;
            for (auto& [id, object] : m_parent.current_scene().objects()) {
                mvp.model = object.transform();
                vk.uniform_buffers[vk.sync.current_frame + ubo_offset].write_data(&mvp, sizeof(mvp));
                ubo_offset += vk.sync.frames_in_flight;
            }

            return {};
        }
    } // namespace engine
} // namespace arbor