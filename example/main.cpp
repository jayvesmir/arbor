#include "arbor/engine.hpp"

int32_t main(int32_t argc, char** argv) {
    arbor::engine::instance engine;

    arbor::engine::scene scene("main");

    scene.vertex_shader("example/shaders/basic.vert");
    scene.fragment_shader("example/shaders/basic.frag");

    arbor::engine::application_config app_config;
    app_config.window = {
        .title  = "arbor",
        .width  = 1280,
        .height = 720,
    };

    engine.push_scene_and_set_current(scene);

    if (auto res = engine.run(app_config); !res)
        return -1;
}