#include "arbor/components/renderer.hpp"

#include "arbor/model.hpp"
#include "vulkan/vk_enum_string_helper.h"

namespace arbor {
    namespace engine {
        std::expected<void, std::string> renderer::pipeline::make_vk_descriptor_pool_and_sets() {
            if (m_descriptor_set_layout)
                return {};

            VkDescriptorSetLayoutCreateInfo create_info{};

            std::vector<VkDescriptorSetLayoutBinding> layout_bindings(2);
            std::vector<VkDescriptorPoolSize> pool_sizes(2);

            layout_bindings[0].binding = 0;
            layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            layout_bindings[0].descriptorCount = 1;
            layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            layout_bindings[1].binding = 1;
            layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            layout_bindings[1].descriptorCount = 1;
            layout_bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            // every object has a ubo and image sampler for each frame in flight
            const auto n_sets = m_parent.vk.sync.frames_in_flight * m_parent.m_parent.current_scene().objects().size();

            pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            pool_sizes[0].descriptorCount = n_sets;

            pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            pool_sizes[1].descriptorCount = n_sets;

            create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            create_info.bindingCount = layout_bindings.size();
            create_info.pBindings = layout_bindings.data();

            m_parent.m_logger->trace("creating a vulkan descriptor set layout");
            if (auto res = vkCreateDescriptorSetLayout(m_parent.vk.device, &create_info, nullptr, &m_descriptor_set_layout);
                res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to create a descriptor set layout: {}", string_VkResult(res)));

            VkDescriptorPoolCreateInfo pool_create_info{};

            pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_create_info.poolSizeCount = pool_sizes.size();
            pool_create_info.pPoolSizes = pool_sizes.data();
            pool_create_info.maxSets = n_sets;

            m_parent.m_logger->trace("creating a vulkan descriptor pool");
            if (auto res = vkCreateDescriptorPool(m_parent.vk.device, &pool_create_info, nullptr, &m_descriptor_pool);
                res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to create a descriptor pool: {}", string_VkResult(res)));

            std::vector<VkDescriptorSetLayout> descriptor_set_layouts(n_sets, m_descriptor_set_layout);

            VkDescriptorSetAllocateInfo allocation_info{};

            allocation_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocation_info.descriptorPool = m_descriptor_pool;
            allocation_info.descriptorSetCount = n_sets;
            allocation_info.pSetLayouts = descriptor_set_layouts.data();

            m_descriptor_sets.resize(n_sets);

            m_parent.m_logger->trace("allocating {} vulkan descriptor sets", n_sets);
            if (auto res = vkAllocateDescriptorSets(m_parent.vk.device, &allocation_info, m_descriptor_sets.data());
                res != VK_SUCCESS)
                return std::unexpected(fmt::format("failed to allocate descriptor sets: {}", string_VkResult(res)));

            for (auto i = 0ull; i < n_sets; i++) {
                VkDescriptorBufferInfo buffer_info{};
                VkDescriptorImageInfo image_info{};

                std::vector<VkWriteDescriptorSet> writes(2);

                buffer_info.buffer = *m_parent.vk.uniform_buffers[i].buffer();
                buffer_info.offset = 0;
                buffer_info.range = sizeof(engine::detail::mvp);

                image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                // TODO: actually handle properly
                const auto texture_id = m_parent.m_textures.begin()->first;
                image_info.imageView = m_parent.m_textures.at(texture_id).at(engine::texture::albedo).image_view();
                image_info.sampler = m_parent.m_textures.at(texture_id).at(engine::texture::albedo).sampler();

                writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writes[0].dstSet = m_descriptor_sets[i];
                writes[0].dstBinding = 0;
                writes[0].dstArrayElement = 0;
                writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                writes[0].descriptorCount = 1;
                writes[0].pBufferInfo = &buffer_info;

                writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writes[1].dstSet = m_descriptor_sets[i];
                writes[1].dstBinding = 1;
                writes[1].dstArrayElement = 0;
                writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                writes[1].descriptorCount = 1;
                writes[1].pImageInfo = &image_info;

                vkUpdateDescriptorSets(m_parent.vk.device, writes.size(), writes.data(), 0, nullptr);
            }

            return {};
        }
    } // namespace engine
} // namespace arbor