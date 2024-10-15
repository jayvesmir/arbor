#include "spdlog/spdlog.h"

import engine;

int32_t main(int32_t argc, char** argv) {
    arbor::engine::instance engine;

    auto res = engine.create_window(1280, 720, "arbor");
    if (!res) {
        spdlog::error("{}", res.error());
        return -1;
    }
}