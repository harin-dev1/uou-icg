#version 460 core

in vec3 dir;
out vec4 outColor;

uniform samplerCube cubemap;

void main() {
    outColor = texture(cubemap, dir);
}