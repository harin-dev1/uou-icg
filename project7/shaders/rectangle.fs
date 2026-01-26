#version 460 core

out vec4 outColor;
in vec4 fragPositionLightView;
uniform sampler2DShadow shadowMap;
void main() {
    vec3 baseColor = vec3(0.8f, 0.2f, 0.2f);
    vec3 ambient = 0.1 * baseColor;
    float shadow = textureProj(shadowMap, fragPositionLightView);
    outColor = vec4(ambient + shadow * 0.9 * baseColor, 1.0);
}

