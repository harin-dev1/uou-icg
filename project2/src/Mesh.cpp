#include "Mesh.h"
#include "Matrix4x4.h"
#include <fstream>
#include <sstream>
#include <iostream>

Mesh::Mesh(const std::string& filename, GLuint shader_program) {
    load_mesh(filename);
    glCreateVertexArrays(1, &m_vao);
    glCreateBuffers(1, &m_vbo);
    glNamedBufferStorage(m_vbo, m_vertices.size() * sizeof(Vec3f), m_vertices.data(), 0);
    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(Vec3f));
    glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(m_vao, 0, 0);
    glEnableVertexArrayAttrib(m_vao, 0);
    m_shader_program = shader_program;
    m_mvp_uniform_location = glGetUniformLocation(m_shader_program, "mvp");
    calculate_bounding_box_center();
}

Mesh::~Mesh() {
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

bool Mesh::load_mesh(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file: " << filename << std::endl;
        return false;
   }
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type, a, b, c, d;
        iss >> type;
        if (type == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            m_vertices.push_back(Vec3f(x, y, z));
        }
    }
    return true;
}

void Mesh::calculate_bounding_box_center() {
    Vec3f min_point(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec3f max_point(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    for (const auto& vertex : m_vertices) {
        min_point = Vec3f::min(min_point, vertex);
        max_point = Vec3f::max(max_point, vertex);
    }
    m_bounding_box_center = (min_point + max_point) / 2.0f;
}

Vec3f Mesh::get_bounding_box_center() const {
    return m_bounding_box_center;
}

void Mesh::draw(const Matrix4x4& mvp) {
    glUseProgram(m_shader_program);
    glUniformMatrix4fv(m_mvp_uniform_location, 1, GL_TRUE, mvp.data());
    glBindVertexArray(m_vao);
    glDrawArrays(GL_POINTS, 0, m_vertices.size());
}

