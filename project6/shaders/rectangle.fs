#version 460 core

in vec3 positionWorld;
in vec3 normalWorld;
out vec4 outColor;
uniform samplerCube cubemap;
uniform vec3 cameraPos;
void main() {
    vec3 dir = normalize(positionWorld - cameraPos);
    vec3 R = reflect(dir, normalWorld);
    outColor = texture(cubemap, R);
}