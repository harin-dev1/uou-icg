#version 460 core

layout(location = 0) in vec3 vpos;
layout(location = 1) in vec2 tcoord;
uniform mat4 mvp;
uniform mat4 mv_matrix;

out vec3 fragPos;
out vec2 fragTCoord;
void main() {
    fragPos = (mv_matrix * vec4(vpos, 1.0)).xyz;
    gl_Position = mvp * vec4(vpos, 1.0);
    fragTCoord = tcoord;
}