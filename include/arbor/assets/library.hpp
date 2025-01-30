#pragma once

#include <unordered_map>

#include "arbor/assets/model.hpp"
#include "arbor/assets/material.hpp"
#include "arbor/types.hpp"

namespace arbor {
    namespace assets {
        class library {
          public:
            struct entry {
                assets::model_3d model;
                assets::material material;
            };

          private:
            std::unordered_map<uint64_t, library::entry> m_entries;

          public:
            constexpr auto& entries() { return m_entries; }
            constexpr auto& entries() const { return m_entries; }

            constexpr auto& operator[](uint64_t id) { return m_entries[id]; }
            constexpr auto& at(uint64_t id) const { return m_entries.at(id); }
        };
    } // namespace assets
} // namespace arbor