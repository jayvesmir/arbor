module;

export module engine.components;

import arbor;

namespace arbor {
    namespace engine {
        export class component {
          public:
            virtual ~component() = 0;
        };
    } // namespace engine
} // namespace arbor