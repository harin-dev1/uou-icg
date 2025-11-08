#version 460 core

out vec4 outColor;
in vec3 Normal;
in vec3 Position;
uniform samplerCube skybox;
uniform vec3 cameraPos;
void main() {
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    outColor = vec4(texture(skybox, R).rgb, 1.0);
}