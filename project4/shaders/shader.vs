#version 460 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 normal;
out vec3 fragColor;
uniform mat4 mvp;
uniform mat4 mv;

void main() {
    gl_Position = mvp * vec4(pos, 1.0);
    fragColor = (mv * vec4(normal, 0.0)).xyz;
}