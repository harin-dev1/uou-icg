#version 460 core

out vec4 outColor;
in vec4 fragPositionLightView;
uniform sampler2DShadow shadowMap;
void main() {
    outColor = vec4(0.8f, 0.2f, 0.2f, 1.0f);
    outColor *= textureProj(shadowMap, fragPositionLightView);
}

