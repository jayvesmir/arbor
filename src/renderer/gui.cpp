#include "arbor/components/renderer.hpp"

#include "fmt/format.h"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"
#include <vulkan/vulkan_core.h>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/matrix_decompose.hpp"

namespace arbor {
    namespace engine {
        std::expected<void, std::string> renderer::init_imgui() {
            if (m_gui.imgui_ctx)
                ImGui::DestroyContext(m_gui.imgui_ctx);

            m_gui.imgui_ctx = ImGui::CreateContext();
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard;

            if (!ImGui_ImplSDL3_InitForVulkan(m_engine.window().sdl_handle()))
                return std::unexpected(fmt::format("failed to initialize ImGui"));

            ImGui_ImplVulkan_InitInfo init_info{};

            init_info.Instance = vk.instance;
            init_info.PhysicalDevice = vk.physical_device.handle;
            init_info.Device = vk.device;
            init_info.QueueFamily = vk.physical_device.queue_family_indices.graphics_family;
            init_info.Queue = vk.graphics_queue;
            init_info.DescriptorPoolSize = 1;
            init_info.Subpass = 0;
            init_info.MinImageCount = vk.sync.frames_in_flight;
            init_info.ImageCount = vk.sync.frames_in_flight;
            init_info.MSAASamples = vk.config.sample_count;
            init_info.RenderPass = m_pipelines.back().render_pass();

            if (!ImGui_ImplVulkan_Init(&init_info))
                return std::unexpected(fmt::format("failed to initialize ImGui"));

            if (!ImGui_ImplVulkan_CreateFontsTexture())
                return std::unexpected(fmt::format("failed to initialize ImGui"));

            m_logger->debug("initialized ImGui");

            return {};
        }

        std::expected<void, std::string> renderer::draw_gui() {
            static std::unordered_map<const char*, VkPresentModeKHR> present_modes = {
                {"immediate", VK_PRESENT_MODE_IMMEDIATE_KHR},
                {"mailbox", VK_PRESENT_MODE_MAILBOX_KHR},
                {"fifo", VK_PRESENT_MODE_FIFO_KHR},
                {"fifo (relaxed)", VK_PRESENT_MODE_FIFO_RELAXED_KHR},
                {"shared demand", VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR},
                {"shared continuous refresh", VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR},
                {"latest ready", VK_PRESENT_MODE_FIFO_LATEST_READY_EXT},
            };

            static const char* const present_modes_opt[] = {
                "immediate", "mailbox", "fifo", "fifo (relaxed)", "shared demand", "shared continuous refresh", "latest ready",
            };

            static int32_t current_present_mode_idx = 1;

            static std::unordered_map<const char*, VkSampleCountFlagBits> msaa_sample_counts = {
                {"2x", VK_SAMPLE_COUNT_2_BIT},
                {"4x", VK_SAMPLE_COUNT_4_BIT},
                {"8x", VK_SAMPLE_COUNT_8_BIT},
            };

            static const char* const msaa_options[] = {"2x", "4x", "8x"};

            static int32_t current_msaa_config_idx = 2;

            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            auto& io = ImGui::GetIO();

            ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

            ImGui::SetNextWindowPos({50, 50}, ImGuiCond_Once);

            ImGui::Begin("arbor menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

            ImGui::SeparatorText("statistics");

            ImGui::Text("scene: %s", m_engine.current_scene().name().c_str());
            ImGui::Text("frametime: %.03f ms (%.03f ms avg.)", m_engine.frame_time_ms(), 1000.f / io.Framerate);
            ImGui::Text("framerate: %.03f (%.03f avg.)", 1000.0f / m_engine.frame_time_ms(), io.Framerate);
            ImGui::Text("frames drawn: %llu", m_engine.frame_count());
            ImGui::Text("objects: %zu (%zu drawable)", m_engine.current_scene().objects().size(),
                        m_engine.current_scene().drawable_objects().size());

            ImGui::SeparatorText("info");

            {
                auto rotation = glm::degrees(m_engine.current_scene().camera().rotation());
                auto position = m_engine.current_scene().camera().position();

                ImGui::Text("camera position [XYZ]: (%.03f, %.03f, %.03f)", position.x, position.y, position.z);
                ImGui::Text("camera rotation [XYZ]: (%.03f°, %.03f°, %.03f°)", rotation.x, rotation.y, rotation.z);

                ImGui::Separator();

                ImGui::Text("mouse position: [XY]: (%.00f, %.00f)", m_engine.input_manager().mouse_position().x,
                            m_engine.input_manager().mouse_position().y);
                ImGui::Text("mouse delta: [XY]: (%.00f, %.00f)", m_engine.input_manager().mouse_delta().x,
                            m_engine.input_manager().mouse_delta().y);
            }

            ImGui::SeparatorText("config");

            if (ImGui::Combo("presentation mode", &current_present_mode_idx, present_modes_opt,
                             IM_ARRAYSIZE(present_modes_opt))) {
                vk.config.present_mode = present_modes[present_modes_opt[current_present_mode_idx]];
                vk.deferred_swapchain_reload = true;
            }

            if (ImGui::Combo("MSAA", &current_msaa_config_idx, msaa_options, IM_ARRAYSIZE(msaa_options))) {
                vk.config.sample_count = msaa_sample_counts[msaa_options[current_present_mode_idx]];
                vk.deferred_swapchain_reload = true;
            }

            if (m_engine.current_scene().controls().size() != 0) {
                ImGui::SeparatorText("scene controls");

                for (auto& [label, control] : m_engine.current_scene().controls())
                    control->imgui_draw(label);
            }

            ImGui::End();

            ImGui::Render();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vk.command_buffers[vk.sync.current_frame]);
            return {};
        }
    } // namespace engine
} // namespace arbor