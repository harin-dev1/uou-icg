#version 460 core

layout(location = 0) in vec3 position;

uniform mat4 mvp;
uniform mat4 shadowMVP;

out vec4 fragPositionLightView;

void main() {
    gl_Position = mvp * vec4(position, 1.0f);
    fragPositionLightView = shadowMVP * vec4(position, 1.0f);
}

