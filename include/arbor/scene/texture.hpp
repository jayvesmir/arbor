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

            enum etype {
                invalid = -1,
                albedo = 0,
            };

          private:
            etype m_type;

            int32_t m_width, m_height;
            std::vector<pixel_rgba> m_pixels;

            std::optional<std::filesystem::path> m_source;

          public:
            texture(etype type = texture::etype::albedo) : m_source(), m_type(type) {}
            texture(const std::filesystem::path& source, etype type = texture::etype::albedo) : m_source(source), m_type(type) {}

            std::expected<void, std::string> load();
            std::expected<void, std::string> load(int32_t width, int32_t height, const texture::pixel_rgba& color = {0, 0, 0, 0});

            constexpr auto type() const { return m_type; }
            constexpr auto width() const { return m_width; }
            constexpr auto height() const { return m_height; }
            constexpr auto& pixels() const { return m_pixels; }
        };
    } // namespace engine
} // namespace arbor