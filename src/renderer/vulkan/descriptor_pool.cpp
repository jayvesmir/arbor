#include "arbor/components/renderer.hpp"

#include "arbor/model.hpp"
#include "vulkan/vk_enum_string_helper.h"

namespace arbor {
    namespace engine {
        std::expected<void, std::string> renderer::pipeline::make_vk_descriptor_pool_and_sets() {
            if (m_descriptor_set_layout)
                return {};

            VkDescriptorSetLayoutBinding layout_binding{};
            VkDescriptorSetLayoutCreateInfo create_info{};

            layout_binding.binding = 0;
            layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            layout_binding.descriptorCount = 1;

            layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            create_info.bindingCount = 1;
            create_info.pBindings = &layout_binding;

            m_parent.m_logger->trace("creating a vulkan descriptor set layout");
            if (auto res = vkCreateDescriptorSetLayout(m_parent.vk.device, &create_info, nullptr, &m_descriptor_set_layout);
                res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to create a descriptor set layout: {}", string_VkResult(res)));

            VkDescriptorPoolSize pool_size{};
            VkDescriptorPoolCreateInfo pool_create_info{};

            pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            pool_size.descriptorCount = m_parent.vk.sync.frames_in_flight;

            pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_create_info.poolSizeCount = 1;
            pool_create_info.pPoolSizes = &pool_size;
            pool_create_info.maxSets = m_parent.vk.sync.frames_in_flight;

            m_parent.m_logger->trace("creating a vulkan descriptor pool");
            if (auto res = vkCreateDescriptorPool(m_parent.vk.device, &pool_create_info, nullptr, &m_descriptor_pool);
                res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to create a descriptor pool: {}", string_VkResult(res)));

            std::vector<VkDescriptorSetLayout> descriptor_set_layouts(m_parent.vk.sync.frames_in_flight, m_descriptor_set_layout);

            VkDescriptorSetAllocateInfo allocation_info{};

            allocation_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocation_info.descriptorPool = m_descriptor_pool;
            allocation_info.descriptorSetCount = m_parent.vk.sync.frames_in_flight;
            allocation_info.pSetLayouts = descriptor_set_layouts.data();

            m_descriptor_sets.resize(m_parent.vk.sync.frames_in_flight);

            m_parent.m_logger->trace("allocating vulkan descriptor sets");
            if (auto res = vkAllocateDescriptorSets(m_parent.vk.device, &allocation_info, m_descriptor_sets.data());
                res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to allocate descriptor sets: {}", string_VkResult(res)));

            for (auto i = 0ull; i < m_parent.vk.sync.frames_in_flight; i++) {
                VkDescriptorBufferInfo buffer_info{};
                VkWriteDescriptorSet write{};

                buffer_info.buffer = *m_parent.vk.uniform_buffers[i].buffer();
                buffer_info.offset = 0;
                buffer_info.range = sizeof(engine::mvp_ubo);

                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = m_descriptor_sets[i];
                write.dstBinding = 0;
                write.dstArrayElement = 0;
                write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write.descriptorCount = 1;
                write.pBufferInfo = &buffer_info;

                vkUpdateDescriptorSets(m_parent.vk.device, 1, &write, 0, nullptr);
            }

            return {};
        }
    } // namespace engine
} // namespace arbor