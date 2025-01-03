#include "arbor/components/renderer.hpp"

#include "fmt/format.h"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"

namespace arbor {
    namespace engine {
        std::expected<void, std::string> renderer::init_imgui() {
            if (m_gui.imgui_ctx)
                ImGui::DestroyContext(m_gui.imgui_ctx);

            m_gui.imgui_ctx = ImGui::CreateContext();
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard;

            if (!ImGui_ImplSDL3_InitForVulkan(m_parent.window().sdl_handle()))
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
            init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
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

            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            auto& io = ImGui::GetIO();

            ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

            ImGui::SetNextWindowPos({50, 50}, ImGuiCond_Once);

            ImGui::Begin("arbor menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

            ImGui::SeparatorText("statistics");

            ImGui::Text("scene: %s", m_parent.current_scene().name().c_str());
            ImGui::Text("frametime: %.03f ms", m_parent.frame_time_ms());
            ImGui::Text("framerate: %.03f", 1000.0f / m_parent.frame_time_ms());
            ImGui::Text("avg. framerate: %.03f", io.Framerate);

            ImGui::SeparatorText("config");

            if (ImGui::Combo("presentation mode", &current_present_mode_idx, present_modes_opt,
                             IM_ARRAYSIZE(present_modes_opt))) {
                vk.config.present_mode = present_modes[present_modes_opt[current_present_mode_idx]];
                vk.deferred_swapchain_reload = true;
            }

            ImGui::End();

            ImGui::Render();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vk.command_buffers[vk.sync.current_frame]);
            return {};
        }
    } // namespace engine
} // namespace arbor