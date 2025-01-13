#include "arbor/engine.hpp"

#include <print>

void init(arbor::engine::instance& engine) {
    arbor::engine::scene scene("main");

    scene.vertex_shader("example/shaders/basic.vert");
    scene.fragment_shader("example/shaders/basic.frag");

    //                                   blah blah blah
    auto plane_id = scene.create_object().value();

    scene.asset_library()[plane_id].model.vertices = {
        {{-1.0f, -1.0f, 0.0f}, {1.0f, 0.25f, 0.25f}, {1.0f, 0.0f}},
        {{1.0f, 1.0f, 0.0f}, {0.25f, 1.0f, 0.25f}, {0.0f, 1.0f}},
        {{1.0f, -1.0f, 0.0f}, {0.25f, 0.25f, 1.0f}, {0.0f, 0.0f}},
        {{-1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
    };

    scene.asset_library()[plane_id].model.indices = {
        0, 2, 1, 1, 3, 0,
    };

    scene.asset_library()[plane_id].textures[arbor::engine::texture::albedo] = {"assets/kitty.jpg"};

    engine.push_scene_and_set_current(scene);
}

void update(arbor::engine::instance& engine) {
    if (engine.frame_count() == 0) {
        std::println("first frame!!!!");
    }
}

int32_t main(int32_t argc, char** argv) {
    arbor::engine::instance engine;

    arbor::engine::application_config app_config;
    app_config.window = {
        .title = "arbor",
        .width = 1280,
        .height = 720,
    };

    app_config.callbacks.on_init = init;
    app_config.callbacks.on_update = update;

    if (auto res = engine.run(app_config); !res)
        return -1;
}