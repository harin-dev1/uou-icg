#version 460 core

layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 normal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec3 Normal;
out vec3 Position;
void main() {
    Normal = mat3(transpose(inverse(model))) * normal;
    Position = vec3(model * vec4(vert, 1.0f));
    gl_Position = projection * view * vec4(Position, 1.0f);
}