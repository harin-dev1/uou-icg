#version 460 core
out vec4 outColor;

in vec3 fragColor;  // from vertex shader

void main() {    
    outColor = vec4(fragColor, 1.0);
}