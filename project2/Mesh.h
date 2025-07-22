#ifndef MESH_H
#define MESH_H

#include <GL/glew.h>
#include <vector>
#include <string>

struct Vertex {
    GLfloat x, y, z;
};

class Mesh {
    public:
        Mesh(const std::string& filename);
        ~Mesh();
        void print();
        void draw();
    private:
        std::vector<Vertex> m_vertices;
        std::vector<GLuint> m_indices;
        GLuint m_vbo;
        GLuint m_ebo;
        GLuint m_vao;
        GLuint m_shader_program;
        void setup_gl_objects();
        void setup_shader();
        void load_mesh(const std::string& filename);
};

#endif // MESH_H 