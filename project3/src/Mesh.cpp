#include "Mesh.h"
#include "Matrix4x4.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <unordered_map>

Mesh::Mesh(const std::string& filename, GLuint shader_program) {
    load_mesh(filename);
    glCreateVertexArrays(1, &m_vao);
    glCreateBuffers(1, &m_vbo);
    glCreateBuffers(1, &m_ibo);
    glNamedBufferStorage(m_vbo, m_vertices.size() * sizeof(Vertex), m_vertices.data(), 0);
    glNamedBufferStorage(m_ibo, m_indices.size() * sizeof(int), m_indices.data(), 0);
    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(Vertex));
    glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(m_vao, 0, 0);
    glEnableVertexArrayAttrib(m_vao, 0);
    glVertexArrayAttribFormat(m_vao, 1, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(m_vao, 1, 0);
    glEnableVertexArrayAttrib(m_vao, 1);
    glVertexArrayElementBuffer(m_vao, m_ibo);
    m_shader_program = shader_program;
    m_mvp_uniform_location = glGetUniformLocation(m_shader_program, "mvp");
    m_mv_uniform_location = glGetUniformLocation(m_shader_program, "mv");
    m_light_pos_uniform_location = glGetUniformLocation(m_shader_program, "light_position");
    m_light_intensity_ambient_uniform_location = glGetUniformLocation(m_shader_program, "light_intensity_ambient");
    m_light_intensity_diffuse_uniform_location = glGetUniformLocation(m_shader_program, "light_intensity_diffuse");
    m_kd_uniform_location = glGetUniformLocation(m_shader_program, "kd");
    m_ks_uniform_location = glGetUniformLocation(m_shader_program, "ks");
    m_view_matrix_uniform_location = glGetUniformLocation(m_shader_program, "view_matrix");
    calculate_bounding_box_center();
}

Mesh::~Mesh() {
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

bool Mesh::load_mesh(const std::string& filename) {
    std::vector<Vec3f> positions;
    std::vector<Vec3f> normals;
    std::unordered_map<std::string, int> indices;
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
            positions.push_back(Vec3f(x, y, z));
            m_vertices.push_back(Vertex{Vec3f(x, y, z)});
        }
        if (type == "vn") {
            float x, y, z;
            iss >> x >> y >> z;
            normals.push_back(Vec3f(x, y, z));
        }
        // f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 ...
        if (type == "f") {
            std::regex regex("((\\d+)/(\\d+)/(\\d+)) ((\\d+)/(\\d+)/(\\d+)) ((\\d+)/(\\d+)/(\\d+)) ((\\d+)/(\\d+)/(\\d+))");
            std::smatch match;
            if (std::regex_search(line, match, regex)) {
                if (indices.find(match[1].str()) == indices.end()) {
                    indices[match[1].str()] = m_vertices.size();
                    m_vertices.push_back(Vertex{positions[std::stoi(match[2].str()) - 1],
                        normals[std::stoi(match[4].str()) - 1]});
                }
                if (indices.find(match[5].str()) == indices.end()) {
                    indices[match[5].str()] = m_vertices.size();
                    m_vertices.push_back(Vertex{positions[std::stoi(match[6].str()) - 1],
                        normals[std::stoi(match[8].str()) - 1]});
                }
                if (indices.find(match[9].str()) == indices.end()) {
                    indices[match[9].str()] = m_vertices.size();
                    m_vertices.push_back(Vertex{positions[std::stoi(match[10].str()) - 1],
                        normals[std::stoi(match[12].str()) - 1]});
                }
                if (indices.find(match[13].str()) == indices.end()) {
                    indices[match[13].str()] = m_vertices.size();
                    m_vertices.push_back(Vertex{positions[std::stoi(match[14].str()) - 1],
                        normals[std::stoi(match[16].str()) - 1]});
                }
                m_indices.push_back(indices[match[1].str()]);
                m_indices.push_back(indices[match[5].str()]);
                m_indices.push_back(indices[match[9].str()]);
                m_indices.push_back(indices[match[1].str()]);
                m_indices.push_back(indices[match[9].str()]);
                m_indices.push_back(indices[match[13].str()]);
            }
        }
    }
    std::cout << "positions.size(): " << positions.size() << std::endl;
    std::cout << "normals.size(): " << normals.size() << std::endl;
    return true;
}

void Mesh::calculate_bounding_box_center() {
    Vec3f min_point(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec3f max_point(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    for (const auto& vertex : m_vertices) {
        min_point = Vec3f::min(min_point, vertex.position);
        max_point = Vec3f::max(max_point, vertex.position);
    }
    m_bounding_box_center = (min_point + max_point) / 2.0f;
}

Vec3f Mesh::get_bounding_box_center() const {
    return m_bounding_box_center;
}

void Mesh::draw(const Matrix4x4& model, const Matrix4x4& view, const Matrix4x4& projection, const Light& light) {
    glUseProgram(m_shader_program);
    glUniformMatrix4fv(m_mvp_uniform_location, 1, GL_TRUE, (projection * view * model).data());
    glUniformMatrix4fv(m_mv_uniform_location, 1, GL_TRUE, (view * model).data());
    glUniform3fv(m_light_pos_uniform_location, 1, (view * Vec3f(light.position.x, light.position.y, light.position.z)).data());
    glUniform3fv(m_light_intensity_ambient_uniform_location, 1, light.intensity_ambient.data());
    glUniform3fv(m_light_intensity_diffuse_uniform_location, 1, light.intensity_diffuse.data());
    glUniform3fv(m_kd_uniform_location, 1, light.material_diffuse_color.data());
    glUniform3fv(m_ks_uniform_location, 1, light.material_specular_color.data());
    glUniformMatrix4fv(m_view_matrix_uniform_location, 1, GL_TRUE, view.data());
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
}

