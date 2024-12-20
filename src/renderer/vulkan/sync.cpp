#include "arbor/components/renderer.hpp"

#include "vulkan/vk_enum_string_helper.h"

namespace arbor {
    namespace engine {
        std::expected<void, std::string> renderer::make_sync_objects() {
            VkSemaphoreCreateInfo semaphore_crate_info{};
            VkFenceCreateInfo fence_create_info{};

            vk.sync.signal_semaphores.resize(vk.sync.frames_in_flight);
            vk.sync.wait_semaphores.resize(vk.sync.frames_in_flight);
            vk.sync.in_flight_fences.resize(vk.sync.frames_in_flight);

            semaphore_crate_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            for (auto i = 0ull; i < vk.sync.frames_in_flight; i++) {
                if (auto res = vkCreateSemaphore(vk.device, &semaphore_crate_info, nullptr, &vk.sync.wait_semaphores[i]);
                    res != VK_SUCCESS)
                    return std::unexpected(fmt::format("failed to create vulkan semaphores: {}", string_VkResult(res)));

                if (auto res = vkCreateSemaphore(vk.device, &semaphore_crate_info, nullptr, &vk.sync.signal_semaphores[i]);
                    res != VK_SUCCESS)
                    return std::unexpected(fmt::format("failed to create vulkan semaphores: {}", string_VkResult(res)));

                if (auto res = vkCreateFence(vk.device, &fence_create_info, nullptr, &vk.sync.in_flight_fences[i]);
                    res != VK_SUCCESS)
                    return std::unexpected(fmt::format("failed to create vulkan fences: {}", string_VkResult(res)));
            }

            return {};
        }
    } // namespace engine
} // namespace arbor