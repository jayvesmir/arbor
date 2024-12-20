#include "arbor/components/renderer.hpp"

#include "fmt/format.h"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"

namespace arbor {
    namespace engine {
        std::expected<void, std::string> renderer::init_imgui() {
            if (m_gui.imgui_ctx)
                ImGui::DestroyContext(m_gui.imgui_ctx);

            m_gui.imgui_ctx = ImGui::CreateContext();
            ImGui::GetIO();

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
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("scene info");

            ImGui::Text("scene: %s", m_parent.current_scene().name().c_str());
            ImGui::Text("frametime: %.03f ms", m_parent.frame_time_ms());
            ImGui::Text("framerate: %.03f", 1000.0f / m_parent.frame_time_ms());

            ImGui::End();

            ImGui::Render();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vk.command_buffers[vk.sync.current_frame]);
            return {};
        }
    } // namespace engine
} // namespace arbor