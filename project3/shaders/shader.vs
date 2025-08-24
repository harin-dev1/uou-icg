#version 460 core

layout(location=0) in vec3 pos;
layout(location = 1) in vec3 normal;
uniform mat4 mvp;
uniform mat4 mv;
out vec3 fragNormal;
out vec3 fragPosition;

void main() {
    gl_Position = mvp * vec4(pos, 1.0);
    fragNormal = (mv * vec4(normal, 0.0)).xyz;
    fragPosition = (mv * vec4(pos, 1.0)).xyz;
}