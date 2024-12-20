#version 460

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec3 vertex_color;
layout(location = 1) in vec2 texture_coord;

layout(binding = 1) uniform sampler2D texture_sampler;

void main() {
    out_color = texture(texture_sampler, texture_coord);
}