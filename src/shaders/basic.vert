#version 460

vec2 vertices[] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.25),
    vec3(0.0, 1.0, 0.25),
    vec3(0.0, 0.0, 1.0)
);

layout(location = 0) out vec3 vertex_color;

void main() {
    gl_Position = vec4(vertices[gl_VertexIndex], 0.0, 1.0);
    vertex_color = colors[gl_VertexIndex];
}