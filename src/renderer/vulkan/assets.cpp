#include "arbor/components/renderer.hpp"

#include "fmt/format.h"
#include "vulkan/vk_enum_string_helper.h"
#include <vulkan/vulkan_core.h>

namespace arbor {
    namespace engine {
        std::expected<void, std::string> renderer::load_assets() {
            m_logger->debug("loading assets onto GPU");

            for (auto& [id, object] : m_engine.current_scene().objects()) {
                auto& asset_library_entry = m_engine.current_scene().asset_library()[id];

                for (auto& [type, texture] : asset_library_entry.material.textures()) {
                    if (auto res = texture.load(); !res)
                        return res;

                    m_logger->debug("loading a {}x{} texture ({} bytes)", texture.width(), texture.height(),
                                    texture.pixels().size() * sizeof(*texture.pixels().begin()));

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

        std::expected<void, std::string> renderer::texture::load(const assets::texture& source, engine::renderer& renderer) {
            destroy();

            m_width = source.width();
            m_height = source.height();

            if (auto res = renderer.make_image(m_width, m_height, VK_FORMAT_R8G8B8A8_UNORM,
                                               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                               VK_IMAGE_ASPECT_COLOR_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                !res) {
                return std::unexpected(res.error());
            } else {
                auto& [image, view, memory] = *res;
                m_image = image;
                m_image_view = view;
                m_image_memory = memory;
            }

            VkSamplerCreateInfo sampler_create_info{};

            VkMemoryRequirements memory_requirements;
            vkGetImageMemoryRequirements(m_device, m_image, &memory_requirements);

            VkPhysicalDeviceMemoryProperties memory_properties;
            vkGetPhysicalDeviceMemoryProperties(m_physical_device, &memory_properties);

            m_staging_buffer.make(memory_requirements.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_device,
                                  m_physical_device);

            m_staging_buffer.write_data(source.pixels().data(), memory_requirements.size);

            renderer.transition_image_layout(m_image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

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

            renderer.transition_image_layout(m_image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

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
    } // namespace engine
} // namespace arbor