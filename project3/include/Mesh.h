#pragma once

#include "Vector.h"
#include <vector>
#include <string>
#include <GL/glew.h>
#include "Matrix4x4.h"
#include "lighting.h"

struct Vertex {
    Vec3f position;
    Vec3f normal;
};

class Mesh {
    private:
    std::vector<Vertex> m_vertices;
    std::vector<int> m_indices;
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ibo;
    GLuint m_mvp_uniform_location;
    GLuint m_mv_uniform_location;
    GLuint m_light_pos_uniform_location;
    GLuint m_light_intensity_ambient_uniform_location;
    GLuint m_light_intensity_diffuse_uniform_location;
    GLuint m_kd_uniform_location;
    GLuint m_ks_uniform_location;
    GLuint m_shader_program;
    bool load_mesh(const std::string& filename);
    void calculate_bounding_box_center();
    Vec3f m_bounding_box_center;
    
    public:
    Mesh(const std::string& filename, GLuint shader_program);
    ~Mesh();
    void draw(const Matrix4x4& model, const Matrix4x4& view, const Matrix4x4& projection, const Light& light);
    Vec3f get_bounding_box_center() const;
};