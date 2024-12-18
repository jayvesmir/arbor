#include "arbor/scene/texture.hpp"

#include <algorithm>
#include <span>

#include "fmt/format.h"
#include "stb_image.h"

namespace arbor {
    namespace engine {
        std::expected<void, std::string> texture::load() {
            if (!m_source)
                return std::unexpected("missing source for texture");

            int32_t _;

            if (auto raw_data = stbi_load(m_source->c_str(), &m_width, &m_height, &_, 4); raw_data != nullptr) {
                m_pixels.resize(m_width * m_height);
                std::ranges::copy(std::span(reinterpret_cast<texture::pixel_rgba*>(raw_data), m_width * m_height),
                                  m_pixels.begin());
                stbi_image_free(raw_data);
            } else {
                return std::unexpected(fmt::format("stb failed to load '{}'", m_source->string()));
            }

            return {};
        }

        std::expected<void, std::string> texture::load(int32_t width, int32_t height, const texture::pixel_rgba& color) {
            m_width = std::max(0, width);
            m_height = std::max(0, height);

            m_pixels.resize(m_width * m_width);
            std::ranges::fill(m_pixels, color);
            return {};
        }
    } // namespace engine
} // namespace arbor