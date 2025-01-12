#include "arbor/components/renderer.hpp"

#include "fmt/format.h"
#include "vulkan/vk_enum_string_helper.h"
#include <vulkan/vulkan_core.h>

namespace arbor {
    namespace engine {
        std::expected<void, std::string> renderer::load_assets() {
            m_logger->debug("loading assets onto GPU");

            for (auto& [id, object] : m_parent.current_scene().objects()) {
                auto& asset_library_entry = m_parent.current_scene().asset_library()[id];

                for (auto& [type, texture] : asset_library_entry.textures) {
                    if (auto res = texture.load(); !res)
                        return res;

                    m_textures[id][type] = {vk.device, vk.physical_device.handle};

                    if (auto res = m_textures[id][type].load(texture, *this); !res)
                        return res;
                }
            }

            return {};
        }

        renderer::texture::~texture() {
            destroy();
        }

        void renderer::texture::destroy() {
            if (m_sampler) {
                vkDestroySampler(m_device, m_sampler, nullptr);
                m_sampler = VK_NULL_HANDLE;
            }

            if (m_image_view) {
                vkDestroyImageView(m_device, m_image_view, nullptr);
                m_image_view = VK_NULL_HANDLE;
            }

            if (m_image) {
                vkDestroyImage(m_device, m_image, nullptr);
                m_image = VK_NULL_HANDLE;
            }

            if (m_image_memory) {
                vkFreeMemory(m_device, m_image_memory, nullptr);
                m_image_memory = nullptr;
            }
        }

        std::expected<void, std::string> renderer::texture::load(const engine::texture& source, engine::renderer& renderer) {
            destroy();

            m_width = source.width();
            m_height = source.height();

            VkImageCreateInfo create_info{};
            VkImageViewCreateInfo view_create_info{};
            VkSamplerCreateInfo sampler_create_info{};

            create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            create_info.imageType = VK_IMAGE_TYPE_2D;
            create_info.extent.width = m_width;
            create_info.extent.height = m_height;
            create_info.extent.depth = 1;
            create_info.mipLevels = 1;
            create_info.arrayLayers = 1;
            create_info.format = VK_FORMAT_R8G8B8A8_SRGB;
            create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
            create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            create_info.samples = VK_SAMPLE_COUNT_1_BIT;

            if (auto res = vkCreateImage(m_device, &create_info, nullptr, &m_image); res != VK_SUCCESS)
                return std::unexpected(
                    fmt::format("failed to create a vulkan image for texture asset: {}", string_VkResult(res)));

            VkMemoryRequirements memory_requirements;
            vkGetImageMemoryRequirements(m_device, m_image, &memory_requirements);

            VkPhysicalDeviceMemoryProperties memory_properties;
            vkGetPhysicalDeviceMemoryProperties(m_physical_device, &memory_properties);

            auto memory_type_idx = 0u;
            for (memory_type_idx = 0u; memory_type_idx < memory_properties.memoryTypeCount; memory_type_idx++) {
                if ((memory_requirements.memoryTypeBits & (1 << memory_type_idx)) &&
                    (memory_properties.memoryTypes[memory_type_idx].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
                    break;
            }

            VkMemoryAllocateInfo allocation_info{};
            allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocation_info.allocationSize = memory_requirements.size;
            allocation_info.memoryTypeIndex = memory_type_idx;

            if (auto res = vkAllocateMemory(m_device, &allocation_info, nullptr, &m_image_memory); res != VK_SUCCESS)
                return std::unexpected(
                    fmt::format("failed to allocate device memory for texture asset: {}", string_VkResult(res)));

            if (auto res = vkBindImageMemory(m_device, m_image, m_image_memory, 0); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to bind texture memory to vulkan image: {}", string_VkResult(res)));

            view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            view_create_info.image = m_image;
            view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view_create_info.format = VK_FORMAT_R8G8B8A8_SRGB;
            view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            view_create_info.subresourceRange.layerCount = 1;
            view_create_info.subresourceRange.levelCount = 1;

            if (auto res = vkCreateImageView(m_device, &view_create_info, nullptr, &m_image_view); res != VK_SUCCESS)
                return std::unexpected(
                    fmt::format("failed to create a vulkan image view for texture asset: {}", string_VkResult(res)));

            m_staging_buffer.make(memory_requirements.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_device,
                                  m_physical_device);

            m_staging_buffer.write_data(source.pixels().data(), memory_requirements.size);

            transition_layout(renderer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            auto command_buffer = renderer.begin_temporary_command_buffer();
            if (!command_buffer)
                return std::unexpected(command_buffer.error());

            VkBufferImageCopy image_copy_region{};
            image_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_copy_region.imageSubresource.layerCount = 1;
            image_copy_region.imageExtent.width = m_width;
            image_copy_region.imageExtent.height = m_height;
            image_copy_region.imageExtent.depth = 1;

            vkCmdCopyBufferToImage(*command_buffer, *m_staging_buffer.buffer(), m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                                   &image_copy_region);

            vkEndCommandBuffer(*command_buffer);

            if (auto res = renderer.submit_command_buffer(*command_buffer, renderer.vk.graphics_queue); !res)
                return res;

            transition_layout(renderer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            sampler_create_info.magFilter = VK_FILTER_LINEAR;
            sampler_create_info.minFilter = VK_FILTER_LINEAR;
            sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler_create_info.anisotropyEnable = VK_TRUE;
            sampler_create_info.maxAnisotropy = renderer.vk.physical_device.properties.limits.maxSamplerAnisotropy;
            sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;

            if (auto res = vkCreateSampler(m_device, &sampler_create_info, nullptr, &m_sampler); res != VK_SUCCESS)
                return std::unexpected(
                    fmt::format("failed to create a texture sampler for texture asset: {}", string_VkResult(res)));

            return {};
        }

        std::expected<void, std::string>
        renderer::texture::transition_layout(engine::renderer& renderer, VkImageLayout old_layout, VkImageLayout new_layout) {
            VkImageMemoryBarrier memory_barrier{};
            VkPipelineStageFlags src_stage;
            VkPipelineStageFlags dst_stage;

            auto command_buffer = renderer.begin_temporary_command_buffer();
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
            }

            memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            memory_barrier.oldLayout = old_layout;
            memory_barrier.newLayout = new_layout;
            memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            memory_barrier.image = m_image;
            memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            memory_barrier.subresourceRange.levelCount = 1;
            memory_barrier.subresourceRange.layerCount = 1;

            vkCmdPipelineBarrier(*command_buffer, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &memory_barrier);

            vkEndCommandBuffer(*command_buffer);

            return renderer.submit_command_buffer(*command_buffer, renderer.vk.graphics_queue);
        }
    } // namespace engine
} // namespace arbor