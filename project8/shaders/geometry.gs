#version 460 core
layout(triangles) in;
layout(line_strip, max_vertices = 4) out;

void main() {
    for (int i = 0; i < 3; i++) {
        gl_Position = gl_in[i].gl_Position;
        gl_Position.z -= 0.01 * gl_Position.w;
        EmitVertex();
    }
    gl_Position = gl_in[0].gl_Position;
    gl_Position.z -= 0.01 * gl_Position.w;
    EmitVertex();
}
