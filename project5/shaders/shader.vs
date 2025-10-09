#version 460 core
layout(location=0) in vec3 vcoord;
layout(location=1) in vec2 tcoord;
out vec2 fragTexCoord;
uniform mat4 mvp;

void main() {
    gl_Position = mvp * vec4(vcoord, 1.0f);
    fragTexCoord = tcoord;
}