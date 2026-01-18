#version 460 core

layout(location = 0) in vec3 vpos;
layout(location = 1) in vec3 vnormal;
uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 mshadow;
out vec3 fragNormalView;
out vec3 fragPositionView;
out vec4 fragPositionLightView;

void main() {
    gl_Position = mvp * vec4(vpos, 1.0f);
    fragNormalView = mat3(mv) * vnormal;
    fragPositionView = (mv * vec4(vpos, 1.0)).xyz;
    fragPositionLightView = (mshadow * vec4(vpos, 1.0));
}