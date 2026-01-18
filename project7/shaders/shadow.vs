#version 460 core

uniform mat4 shadowMVP;
layout(location = 0) in vec3 position;

void main() {
    gl_Position = shadowMVP * vec4(position, 1.0);
}