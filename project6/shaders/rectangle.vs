#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 positionWorld;
out vec3 normalWorld;
void main() {
    positionWorld = vec3(model * vec4(position, 1.0f));
    normalWorld = transpose(inverse(mat3(model))) * normal;
    gl_Position =  projection * view * vec4(positionWorld, 1.0f);
}