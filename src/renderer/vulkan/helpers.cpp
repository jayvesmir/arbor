#include "arbor/components/renderer.hpp"

#include "vulkan/vk_enum_string_helper.h"

namespace arbor {
    namespace engine {
        std::expected<std::tuple<VkImage, VkImageView, VkDeviceMemory>, std::string>
        renderer::make_image(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage,
                             VkImageAspectFlags aspect_mask, VkMemoryPropertyFlags memory_props) {

            VkImage image;
            VkImageView view;
            VkDeviceMemory memory;

            VkImageCreateInfo create_info{};
            VkImageViewCreateInfo view_create_info{};
            VkSamplerCreateInfo sampler_create_info{};

            create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            create_info.imageType = VK_IMAGE_TYPE_2D;
            create_info.extent.width = width;
            create_info.extent.height = height;
            create_info.extent.depth = 1;
            create_info.mipLevels = 1;
            create_info.arrayLayers = 1;
            create_info.format = format;
            create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
            create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            create_info.usage = usage;
            create_info.samples = VK_SAMPLE_COUNT_1_BIT;

            if (auto res = vkCreateImage(vk.device, &create_info, nullptr, &image); res != VK_SUCCESS)
                return std::unexpected(
                    fmt::format("failed to create a vulkan image for texture asset: {}", string_VkResult(res)));

            VkMemoryRequirements memory_requirements;
            vkGetImageMemoryRequirements(vk.device, image, &memory_requirements);

            VkPhysicalDeviceMemoryProperties memory_properties;
            vkGetPhysicalDeviceMemoryProperties(vk.physical_device.handle, &memory_properties);

            auto memory_type_idx = 0u;
            for (memory_type_idx = 0u; memory_type_idx < memory_properties.memoryTypeCount; memory_type_idx++) {
                if ((memory_requirements.memoryTypeBits & (1 << memory_type_idx)) &&
                    (memory_properties.memoryTypes[memory_type_idx].propertyFlags & memory_props))
                    break;
            }

            VkMemoryAllocateInfo allocation_info{};
            allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocation_info.allocationSize = memory_requirements.size;
            allocation_info.memoryTypeIndex = memory_type_idx;

            if (auto res = vkAllocateMemory(vk.device, &allocation_info, nullptr, &memory); res != VK_SUCCESS)
                return std::unexpected(
                    fmt::format("failed to allocate device memory for texture asset: {}", string_VkResult(res)));

            if (auto res = vkBindImageMemory(vk.device, image, memory, 0); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to bind texture memory to vulkan image: {}", string_VkResult(res)));

            view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            view_create_info.image = image;
            view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view_create_info.format = format;
            view_create_info.subresourceRange.aspectMask = aspect_mask;
            view_create_info.subresourceRange.layerCount = 1;
            view_create_info.subresourceRange.levelCount = 1;

            if (auto res = vkCreateImageView(vk.device, &view_create_info, nullptr, &view); res != VK_SUCCESS)
                return std::unexpected(
                    fmt::format("failed to create a vulkan image view for texture asset: {}", string_VkResult(res)));

            return {{image, view, memory}};
        }

        std::expected<void, std::string> renderer::transition_image_layout(VkImage image, VkImageAspectFlags aspect_mask,
                                                                           VkImageLayout old_layout, VkImageLayout new_layout) {
            VkImageMemoryBarrier memory_barrier{};
            VkPipelineStageFlags src_stage;
            VkPipelineStageFlags dst_stage;

            auto command_buffer = begin_temporary_command_buffer();
            if (!command_buffer)
                return std::unexpected(command_buffer.error());

            if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                memory_barrier.srcAccessMask = 0;
                memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                       new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
                       new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                memory_barrier.srcAccessMask = 0;
                memory_barrier.dstAccessMask =
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

                src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            }

            memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            memory_barrier.oldLayout = old_layout;
            memory_barrier.newLayout = new_layout;
            memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            memory_barrier.image = image;
            memory_barrier.subresourceRange.aspectMask = aspect_mask;
            memory_barrier.subresourceRange.levelCount = 1;
            memory_barrier.subresourceRange.layerCount = 1;

            vkCmdPipelineBarrier(*command_buffer, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &memory_barrier);

            vkEndCommandBuffer(*command_buffer);

            return submit_command_buffer(*command_buffer, vk.graphics_queue);
        }
    } // namespace engine
} // namespace arbor