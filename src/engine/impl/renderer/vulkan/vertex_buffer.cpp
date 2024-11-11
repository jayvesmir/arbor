#include "engine/components/renderer.hpp"

#include "vulkan/vk_enum_string_helper.h"

namespace arbor {
    namespace engine {
        std::expected<void, std::string> renderer::make_vertex_buffer() {
            VkBufferCreateInfo create_info{};

            create_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            create_info.size        = m_test_vertices.size() * sizeof(m_test_vertices[0]);
            create_info.usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (auto res = vkCreateBuffer(vk.device, &create_info, nullptr, &vk.vertex_buffer); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to create vertex buffer: {}", string_VkResult(res)));

            VkMemoryRequirements memory_requirements;
            vkGetBufferMemoryRequirements(vk.device, vk.vertex_buffer, &memory_requirements);

            VkPhysicalDeviceMemoryProperties memory_properties;
            vkGetPhysicalDeviceMemoryProperties(vk.physical_device.handle, &memory_properties);

            uint32_t memory_type                 = memory_requirements.memoryTypeBits;
            VkMemoryPropertyFlags property_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

            auto memory_type_idx = 0u;
            for (memory_type_idx = 0u; memory_type_idx < memory_properties.memoryTypeCount; memory_type_idx++) {
                if ((memory_type & (1 << memory_type_idx)) &&
                    (memory_properties.memoryTypes[memory_type_idx].propertyFlags & property_flags))
                    break;
            }

            VkMemoryAllocateInfo allocation_info{};

            allocation_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocation_info.allocationSize  = memory_requirements.size;
            allocation_info.memoryTypeIndex = memory_type_idx;

            if (auto res = vkAllocateMemory(vk.device, &allocation_info, nullptr, &vk.vertex_buffer_memory); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to allocate vertex buffer memory: {}", string_VkResult(res)));

            if (auto res = vkBindBufferMemory(vk.device, vk.vertex_buffer, vk.vertex_buffer_memory, 0); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to bind vertex buffer memory: {}", string_VkResult(res)));

            void* data;
            vkMapMemory(vk.device, vk.vertex_buffer_memory, 0, create_info.size, 0, &data);
            std::memcpy(data, m_test_vertices.data(), create_info.size);
            vkUnmapMemory(vk.device, vk.vertex_buffer_memory);

            return {};
        }
    } // namespace engine
} // namespace arbor