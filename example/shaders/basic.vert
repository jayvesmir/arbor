#version 460

layout(binding = 0) uniform uniform_buffer {
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

layout(location = 0) in vec2 vert_position;
layout(location = 1) in vec3 vert_color;

layout(location = 0) out vec3 frag_color;

void main() {
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(vert_position, 0.0, 1.0);
    frag_color  = vert_color;
}