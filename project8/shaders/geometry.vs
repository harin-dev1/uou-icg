#version 460 core
uniform mat4 mvp;
layout(location = 0) in vec3 vpos;

void main() {
    gl_Position = mvp * vec4(vpos, 1.0);
}