module;
#include <expected>
#include <string>

export module engine.components;
import arbor;

namespace arbor {
    namespace engine {
        export class component {
          public:
            enum etype {
                invalid  = -1,
                renderer = 0,
            };

          protected:
            component::etype m_type = etype::invalid;

          public:
            virtual ~component() = 0;

            constexpr auto type() const { return m_type; }

            virtual void shutdown()                         = 0;
            virtual std::expected<void, std::string> init() = 0;
        };
    } // namespace engine
} // namespace arbor