#include "arbor/components/renderer.hpp"

#include "arbor/model.hpp"
#include "vulkan/vk_enum_string_helper.h"
#include <vulkan/vulkan_core.h>

namespace arbor {
    namespace engine {
        std::expected<void, std::string> renderer::device_buffer::make(uint64_t size, VkBufferUsageFlags usage,
                                                                       VkMemoryPropertyFlags properties, VkDevice device,
                                                                       VkPhysicalDevice physical_device, bool keep_mapped) {
            VkBufferCreateInfo create_info{};
            VkMemoryAllocateInfo allocation_info{};

            m_keep_mapped = keep_mapped;
            m_device = device;
            m_physical_device = physical_device;
            m_size = size;

            create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            create_info.size = size;
            create_info.usage = usage;
            create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (auto res = vkCreateBuffer(m_device, &create_info, nullptr, &m_buffer); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to create buffer: {}", string_VkResult(res)));

            VkMemoryRequirements memory_requirements;
            vkGetBufferMemoryRequirements(m_device, m_buffer, &memory_requirements);

            VkPhysicalDeviceMemoryProperties memory_properties;
            vkGetPhysicalDeviceMemoryProperties(m_physical_device, &memory_properties);

            uint32_t memory_type = memory_requirements.memoryTypeBits;

            auto memory_type_idx = 0u;
            for (memory_type_idx = 0u; memory_type_idx < memory_properties.memoryTypeCount; memory_type_idx++) {
                if ((memory_type & (1 << memory_type_idx)) &&
                    (memory_properties.memoryTypes[memory_type_idx].propertyFlags & properties))
                    break;
            }

            allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocation_info.allocationSize = memory_requirements.size;
            allocation_info.memoryTypeIndex = memory_type_idx;

            if (auto res = vkAllocateMemory(m_device, &allocation_info, nullptr, &m_memory); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to allocate buffer memory: {}", string_VkResult(res)));

            if (auto res = vkBindBufferMemory(m_device, m_buffer, m_memory, 0); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to bind buffer memory: {}", string_VkResult(res)));

            if (m_keep_mapped) {
                if (auto res = vkMapMemory(m_device, m_memory, 0, size, 0, &m_mapped); res != VK_SUCCESS)
                    return std::unexpected(fmt::format("failed to map device buffer memory: {}", string_VkResult(res)));
            }

            return {};
        }

        std::expected<void, std::string> renderer::device_buffer::write_data(const void* bytes, uint64_t size,
                                                                             VkQueue transfer_queue, VkCommandPool command_pool) {
            if (!transfer_queue || !command_pool) {
                if (!m_keep_mapped || !m_mapped) {
                    if (auto res = vkMapMemory(m_device, m_memory, 0, size, 0, &m_mapped); res != VK_SUCCESS)
                        return std::unexpected(fmt::format("failed to map device buffer memory: {}", string_VkResult(res)));
                }

                std::memcpy(m_mapped, bytes, size);

                if (!m_keep_mapped) {
                    vkUnmapMemory(m_device, m_memory);
                    m_mapped = nullptr;
                }

                return {};
            }

            renderer::device_buffer staging_buffer;
            staging_buffer.make(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_device,
                                m_physical_device);

            staging_buffer.write_data(bytes, size);

            free();
            make(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 m_device, m_physical_device);

            VkSubmitInfo submit_info{};
            VkCommandBufferBeginInfo begin_info{};
            VkCommandBufferAllocateInfo command_buffer_allocate_info{};

            command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            command_buffer_allocate_info.commandBufferCount = 1;
            command_buffer_allocate_info.commandPool = command_pool;
            command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

            VkCommandBuffer command_buffer;
            if (auto res = vkAllocateCommandBuffers(m_device, &command_buffer_allocate_info, &command_buffer); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to allocate a staging command buffer", string_VkResult(res)));

            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            if (auto res = vkBeginCommandBuffer(command_buffer, &begin_info); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to begin a staging command buffer", string_VkResult(res)));

            VkBufferCopy copy{};
            copy.size = size;

            vkCmdCopyBuffer(command_buffer, staging_buffer.m_buffer, m_buffer, 1, &copy);

            if (auto res = vkEndCommandBuffer(command_buffer); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to end staging command buffer: {}", string_VkResult(res)));

            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &command_buffer;

            if (auto res = vkQueueSubmit(transfer_queue, 1, &submit_info, VK_NULL_HANDLE); res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to stage buffer to GPU: {}", string_VkResult(res)));

            vkQueueWaitIdle(transfer_queue);

            vkFreeCommandBuffers(m_device, command_pool, 1, &command_buffer);

            return {};
        }

        void renderer::device_buffer::free() {
            if (m_mapped) {
                vkUnmapMemory(m_device, m_memory);
                m_mapped = nullptr;
            }

            if (m_buffer && m_device) {
                vkDestroyBuffer(m_device, m_buffer, nullptr);
                m_buffer = VK_NULL_HANDLE;
            }

            if (m_memory && m_device) {
                vkFreeMemory(m_device, m_memory, nullptr);
                m_memory = VK_NULL_HANDLE;
            }
        }

        renderer::device_buffer::~device_buffer() {
            free();
        }

        std::expected<void, std::string> renderer::make_vertex_buffer() {
            uint64_t size = 0;
            std::vector<engine::vertex_3d> vertices;
            for (auto [id, object] : m_engine.current_scene().objects()) {
                size += m_engine.current_scene().asset_library()[id].model.vertices.size() *
                        sizeof(*m_engine.current_scene().asset_library()[id].model.vertices.begin());

                vertices.append_range(m_engine.current_scene().asset_library()[id].model.vertices);
            }

            m_logger->trace("allocating a vertex buffer of {} vertices ({} bytes)", vertices.size(), size);

            if (auto res = vk.vertex_buffer.make(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                 vk.device, vk.physical_device.handle);
                !res) {
                return res;
            }

            if (auto res = vk.vertex_buffer.write_data(vertices.data(), size, vk.graphics_queue, vk.command_pool); !res)
                return res;

            return {};
        }

        std::expected<void, std::string> renderer::make_index_buffer() {
            uint64_t size = 0;
            std::vector<uint32_t> indices;
            for (auto [id, object] : m_engine.current_scene().objects()) {
                size += m_engine.current_scene().asset_library()[id].model.indices.size() *
                        sizeof(*m_engine.current_scene().asset_library()[id].model.indices.begin());

                indices.append_range(m_engine.current_scene().asset_library()[id].model.indices);
            }

            m_logger->trace("allocating an index buffer of {} vertices ({} bytes)", indices.size(), size);

            if (auto res = vk.index_buffer.make(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                vk.device, vk.physical_device.handle);
                !res)
                return res;

            if (auto res = vk.index_buffer.write_data(indices.data(), size, vk.graphics_queue, vk.command_pool); !res)
                return res;

            return {};
        }

        std::expected<void, std::string> renderer::make_uniform_buffers() {
            auto size = sizeof(engine::detail::mvp);

            vk.uniform_buffers.resize(vk.sync.frames_in_flight * m_engine.current_scene().objects().size());

            for (auto& buffer : vk.uniform_buffers) {
                if (auto res = buffer.make(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vk.device,
                                           vk.physical_device.handle, true);
                    !res) {
                    return res;
                }
            }

            return {};
        }
    } // namespace engine
} // namespace arbor