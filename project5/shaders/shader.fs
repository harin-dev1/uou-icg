#version 460 core
out vec4 outColor;
in vec2 fragTexCoord;
uniform sampler2D texture_color;

void main() {
    outColor = texture(texture_color, fragTexCoord) + vec4(0.1f, 0.1f, 0.1f, 0.5f);
}