#include "arbor/components/renderer.hpp"
#include <ranges>

#include "SDL3/SDL_error.h"
#include "SDL3/SDL_vulkan.h"
#include "vulkan/vk_enum_string_helper.h"

namespace arbor {
    namespace engine {
        std::expected<void, std::string> renderer::make_vk_surface() {
            m_logger->trace("creating a window surface");
            if (!SDL_Vulkan_CreateSurface(m_parent.window().sdl_handle(), vk.instance, nullptr, &vk.swapchain.surface))
                return std::unexpected(fmt::format("failed to create window surface: {}", SDL_GetError()));

            return {};
        }

        std::expected<void, std::string> renderer::make_vk_swapchain_and_pipeline() {
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk.physical_device.handle, vk.swapchain.surface,
                                                      &vk.swapchain.surface_capabilities);
            vk.swapchain.extent = vk.swapchain.surface_capabilities.currentExtent;

            uint32_t vk_n = 0;
            std::vector<VkSurfaceFormatKHR> formats;
            vkGetPhysicalDeviceSurfaceFormatsKHR(vk.physical_device.handle, vk.swapchain.surface, &vk_n, nullptr);
            formats.resize(vk_n);
            vkGetPhysicalDeviceSurfaceFormatsKHR(vk.physical_device.handle, vk.swapchain.surface, &vk_n, formats.data());

            std::vector<VkPresentModeKHR> present_modes;
            vkGetPhysicalDeviceSurfacePresentModesKHR(vk.physical_device.handle, vk.swapchain.surface, &vk_n, nullptr);
            present_modes.resize(vk_n);
            vkGetPhysicalDeviceSurfacePresentModesKHR(vk.physical_device.handle, vk.swapchain.surface, &vk_n,
                                                      present_modes.data());

            if (formats.empty() || present_modes.empty())
                return std::unexpected("incompatible surface");

            auto format = std::ranges::find_if(formats, [](const VkSurfaceFormatKHR& format) {
                if (format.format == VK_FORMAT_R8G8B8A8_SINT && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    return true;
                return false;
            });

            auto present_mode = std::ranges::find_if(present_modes, [&](const VkPresentModeKHR& mode) {
                if (mode == vk.config.present_mode)
                    return true;
                return false;
            });

            if (format != formats.end())
                vk.swapchain.format = *format;
            else
                vk.swapchain.format = formats.front();

            if (present_mode != present_modes.end())
                vk.swapchain.present_mode = *present_mode;
            else
                vk.swapchain.present_mode = present_modes.front();

            if (m_pipelines.empty())
                if (auto res = make_vk_pipeline(); !res)
                    return res;

            VkSwapchainCreateInfoKHR create_info{};

            create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            create_info.surface = vk.swapchain.surface;
            create_info.minImageCount =
                std::min(vk.swapchain.surface_capabilities.minImageCount + 1, vk.swapchain.surface_capabilities.maxImageCount);

            create_info.imageExtent = vk.swapchain.extent;
            create_info.presentMode = vk.swapchain.present_mode;
            create_info.imageFormat = vk.swapchain.format.format;
            create_info.imageColorSpace = vk.swapchain.format.colorSpace;

            create_info.imageArrayLayers = 1;
            create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            if (vk.physical_device.queue_family_indices.graphics_family ==
                vk.physical_device.queue_family_indices.present_family) {
                create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            } else {
                create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                create_info.queueFamilyIndexCount = 2;
                create_info.pQueueFamilyIndices = reinterpret_cast<uint32_t*>(&vk.physical_device.queue_family_indices);
            }

            create_info.clipped = VK_TRUE;
            create_info.preTransform = vk.swapchain.surface_capabilities.currentTransform;
            create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

            if (auto res = vkCreateSwapchainKHR(vk.device, &create_info, nullptr, &vk.swapchain.handle); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to create a swapchain: {}", string_VkResult(res)));

            vkGetSwapchainImagesKHR(vk.device, vk.swapchain.handle, &vk_n, nullptr);
            vk.swapchain.images.resize(vk_n);
            vk.swapchain.image_views.resize(vk_n);
            vk.swapchain.framebuffers.resize(vk_n);
            vkGetSwapchainImagesKHR(vk.device, vk.swapchain.handle, &vk_n, vk.swapchain.images.data());

            m_pipelines.back().reload();

            m_logger->trace("created a vulkan swapchain with {} images", vk.swapchain.images.size());

            for (auto i = 0ull; i < vk.swapchain.image_views.size(); i++) {
                VkImageViewCreateInfo view_create_info{};
                VkFramebufferCreateInfo framebuffer_create_info{};

                view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                view_create_info.image = vk.swapchain.images[i];
                view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                view_create_info.format = vk.swapchain.format.format;

                view_create_info.subresourceRange.levelCount = 1;
                view_create_info.subresourceRange.layerCount = 1;
                view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

                if (auto res = vkCreateImageView(vk.device, &view_create_info, nullptr, &vk.swapchain.image_views[i]);
                    res != VK_SUCCESS)
                    return std::unexpected(fmt::format("failed to create swapchain image view {}: {}", i, string_VkResult(res)));

                framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebuffer_create_info.renderPass = m_pipelines.back().render_pass();
                framebuffer_create_info.attachmentCount = 1;
                framebuffer_create_info.pAttachments = &vk.swapchain.image_views[i];
                framebuffer_create_info.width = vk.swapchain.extent.width;
                framebuffer_create_info.height = vk.swapchain.extent.height;
                framebuffer_create_info.layers = 1;

                if (auto res = vkCreateFramebuffer(vk.device, &framebuffer_create_info, nullptr, &vk.swapchain.framebuffers[i]);
                    res != VK_SUCCESS)
                    return std::unexpected(fmt::format("failed to create framebuffer {}: {}", i, string_VkResult(res)));
            }

            return {};
        }

        std::expected<uint32_t, std::string> renderer::acquire_image() {
            uint32_t image_idx;
            if (auto res = vkAcquireNextImageKHR(vk.device, vk.swapchain.handle, uint64_t(-1),
                                                 vk.sync.wait_semaphores[vk.sync.current_frame], VK_NULL_HANDLE, &image_idx);
                res != VK_SUCCESS) {
                if (res == VK_ERROR_OUT_OF_DATE_KHR) {
                    resize_viewport();
                    return uint32_t(-1);
                }
                return std::unexpected(fmt::format("failed to acquire a swapchain image: {}", string_VkResult(res)));
            }

            return image_idx;
        }

        std::expected<void, std::string> renderer::reload_swapchain() {
            m_logger->trace("rebuilding swapchain ({}x{})", m_parent.window().width(), m_parent.window().height());

            vkDeviceWaitIdle(vk.device);

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

            if (vk.swapchain.handle && vk.device)
                vkDestroySwapchainKHR(vk.device, vk.swapchain.handle, nullptr);

            return make_vk_swapchain_and_pipeline();
        }
    } // namespace engine
} // namespace arbor