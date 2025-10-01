#version 460 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texCoord;
out vec3 fragColor;
uniform mat4 mvp;
uniform mat4 mv_inv_transpose;
uniform mat4 mv;
out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexCoord;

void main() {
    gl_Position = mvp * vec4(pos, 1.0);
    fragNormal = (mv_inv_transpose * vec4(normal, 0.0)).xyz;
    fragPosition = (mv * vec4(pos, 1.0)).xyz;
    fragTexCoord = texCoord;
}