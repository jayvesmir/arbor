#include "spdlog/spdlog.h"

#include <exception>

import arbor;
import engine;

int32_t main(int32_t argc, char** argv) {
    arbor::engine::instance engine;

    arbor::engine::application_config app_config;
    app_config.window = {
        .title  = "arbor",
        .width  = 1280,
        .height = 720,
    };

    if (auto res = engine.create_app(app_config); !res)
        return -1;
}