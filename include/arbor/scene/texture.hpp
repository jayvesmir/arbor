#pragma once

#include <expected>
#include <filesystem>
#include <optional>
#include <vector>

#include "arbor/types.hpp"

namespace arbor {
    namespace engine {
        class texture {
          public:
            struct pixel_rgba {
                uint8_t r, g, b, a;
            };

          private:
            int32_t m_width, m_height;
            std::vector<pixel_rgba> m_pixels;

            std::optional<std::filesystem::path> m_source;

          public:
            texture() : m_source() {}
            texture(const std::filesystem::path& source) : m_source(source) {}

            std::expected<void, std::string> load();
            std::expected<void, std::string> load(int32_t width, int32_t height, const texture::pixel_rgba& color = {0, 0, 0, 0});

            constexpr auto width() const { return m_width; }
            constexpr auto height() const { return m_height; }
            constexpr auto& pixels() const { return m_pixels; }
        };
    } // namespace engine
} // namespace arbor