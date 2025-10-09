#version 460 core
in vec2 fragTexCoord;
out vec4 outColor;

uniform sampler2D tex_kd;
uniform bool has_texture_kd;

void main() {
    if (has_texture_kd) {
        outColor = texture(tex_kd, fragTexCoord);
    } else {
        outColor = vec4(1.f, 1.f, 1.f, 1.f);
    }
}