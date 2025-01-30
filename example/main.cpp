#include "arbor/assets/model.hpp"
#include "arbor/engine.hpp"
#include "arbor/scene/controls.hpp"
#include "glm/ext/matrix_transform.hpp"

#include <print>

void init(arbor::engine::instance& engine) {
    arbor::scene::instance scene("main");

    scene.vertex_shader("example/shaders/basic.vert");
    scene.fragment_shader("example/shaders/basic.frag");

    scene.camera().translate(glm::vec3(0.0f, 3.0f, 1.0f));
    scene.camera().rotate(glm::vec3(0.0f, -90.0f, 0.0f));

    scene.add_control<arbor::scene::controls::slider_f32>("plane movement speed", 0.0f, 5.0f, 0.0f);
    scene.add_control<arbor::scene::controls::slider_f32>("cube rotation speed", 0.0f, 5.0f, 0.0f);

    {
        //                                   blah blah blah
        auto plane_id = scene.create_object().value();

        scene.asset_library()[plane_id].model = arbor::assets::model_3d::plane(0.5f, 0.5f);
        scene.asset_library()[plane_id].material.textures()[arbor::assets::texture::albedo] = {"assets/kitty0.jpg"};

        scene.objects()[plane_id].callbacks().on_update = [](arbor::engine::instance& engine, uint64_t id) {
            auto& self = engine.current_scene().objects()[id];
            static auto speed = engine.current_scene().control<arbor::scene::controls::slider_f32>("plane movement speed");

            static float position = 0.0f;
            position += engine.frame_time_ms() * (0.005f / 2.0f) * (*speed)->value();

            self.transform() = glm::mat4(1.0f);
            self.transform() =
                glm::translate(self.transform(), glm::vec3(glm::sin(position), glm::cos(position), glm::sin(position * 2) / 2));
        };
    }

    {
        //                                   blah blah blah
        auto cube_id = scene.create_object().value();

        scene.asset_library()[cube_id].model = arbor::assets::model_3d::cube_uv(0.5f, 0.5f, 0.5f);
        scene.asset_library()[cube_id].material.textures()[arbor::assets::texture::albedo] = {"assets/cube.png"};

        scene.objects()[cube_id].callbacks().on_update = [](arbor::engine::instance& engine, uint64_t id) {
            auto& self = engine.current_scene().objects()[id];
            static auto speed = engine.current_scene().control<arbor::scene::controls::slider_f32>("cube rotation speed");

            static float rotation = 0.0f;
            rotation += engine.frame_time_ms() * (0.25f) * (*speed)->value();

            self.transform() = glm::mat4(1.0f);
            self.transform() = glm::rotate(self.transform(), -glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
        };
    }

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