#version 460 core

layout(location = 0) in vec3 vpos;
layout(location = 1) in vec3 vnormal;
uniform mat4 mvp;
out vec3 fragColor;

void main() {
    gl_Position = mvp * vec4(vpos, 1.0f);
    fragColor = vnormal;
}