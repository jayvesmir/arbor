#include "engine/components/renderer.hpp"

#include "vulkan/vk_enum_string_helper.h"
#include <vulkan/vulkan_core.h>

namespace arbor {
    namespace engine {
        std::expected<void, std::string> renderer::device_buffer::make(uint64_t size, VkBufferUsageFlags usage, VkDevice device,
                                                                       VkPhysicalDevice physical_device) {
            VkBufferCreateInfo create_info{};
            VkMemoryAllocateInfo allocation_info{};

            m_device = device;

            create_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            create_info.size        = size;
            create_info.usage       = usage;
            create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (auto res = vkCreateBuffer(m_device, &create_info, nullptr, &m_buffer); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to create buffer: {}", string_VkResult(res)));

            VkMemoryRequirements memory_requirements;
            vkGetBufferMemoryRequirements(m_device, m_buffer, &memory_requirements);

            VkPhysicalDeviceMemoryProperties memory_properties;
            vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

            uint32_t memory_type                 = memory_requirements.memoryTypeBits;
            VkMemoryPropertyFlags property_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

            auto memory_type_idx = 0u;
            for (memory_type_idx = 0u; memory_type_idx < memory_properties.memoryTypeCount; memory_type_idx++) {
                if ((memory_type & (1 << memory_type_idx)) &&
                    (memory_properties.memoryTypes[memory_type_idx].propertyFlags & property_flags))
                    break;
            }

            allocation_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocation_info.allocationSize  = memory_requirements.size;
            allocation_info.memoryTypeIndex = memory_type_idx;

            if (auto res = vkAllocateMemory(m_device, &allocation_info, nullptr, &m_memory); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to allocate buffer memory: {}", string_VkResult(res)));

            if (auto res = vkBindBufferMemory(m_device, m_buffer, m_memory, 0); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to bind buffer memory: {}", string_VkResult(res)));

            return {};
        }

        std::expected<void, std::string> renderer::device_buffer::write_data(const void* bytes, uint64_t size) {
            void* mapped;
            if (auto res = vkMapMemory(m_device, m_memory, 0, size, 0, &mapped); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to map device buffer memory: {}", string_VkResult(res)));

            std::memcpy(mapped, bytes, size);

            vkUnmapMemory(m_device, m_memory);

            return {};
        }

        void renderer::device_buffer::free() {
            if (m_buffer && m_device) {
                vkDestroyBuffer(m_device, m_buffer, nullptr);
                m_buffer = VK_NULL_HANDLE;
            }

            if (m_memory && m_device) {
                vkFreeMemory(m_device, m_memory, nullptr);
                m_memory = VK_NULL_HANDLE;
            }
        }

        std::expected<void, std::string> renderer::make_vertex_buffer() {
            if (auto res = vk.vertex_buffer.make(m_test_vertices.size() * sizeof(m_test_vertices[0]), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                 vk.device, vk.physical_device.handle);
                !res)
                return res;

            if (auto res = vk.vertex_buffer.write_data(m_test_vertices.data(), m_test_vertices.size() * sizeof(m_test_vertices[0])); !res)
                return res;

            return {};
        }
    } // namespace engine
} // namespace arbor