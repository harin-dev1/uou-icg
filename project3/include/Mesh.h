#pragma once

#include "Vector.h"
#include <vector>
#include <string>
#include <GL/glew.h>
#include "Matrix4x4.h"

struct Vertex {
    Vec3f position;
};

class Mesh {
    private:
    std::vector<Vertex> m_vertices;
    std::vector<int> m_indices;
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ibo;
    GLuint m_mvp_uniform_location;
    GLuint m_shader_program;
    bool load_mesh(const std::string& filename);
    void calculate_bounding_box_center();
    Vec3f m_bounding_box_center;
    
    public:
    Mesh(const std::string& filename, GLuint shader_program);
    ~Mesh();
    void draw(const Matrix4x4& mvp);
    Vec3f get_bounding_box_center() const;
};