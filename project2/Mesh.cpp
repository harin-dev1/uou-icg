#include "Mesh.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <iomanip>

void Mesh::draw() {
    glDrawArrays(GL_POINTS, 0, m_vertices.size());
}

void Mesh::print() {
    std::cout << "Vertices: " << m_vertices.size() << std::endl;
    for (uint32_t i = 0; i < m_vertices.size(); i++) {
        std::cout <<"v  " << std::fixed << std::setprecision(4) << m_vertices[i].x << " " << std::fixed << std::setprecision(4) << m_vertices[i].y << " " << std::fixed << std::setprecision(4) << m_vertices[i].z << std::endl;
    }
    std::cout << "Indices: " << m_indices.size() << std::endl;
    for (uint32_t i = 0; i < m_indices.size(); i++) {
        std::cout << m_indices[i] << std::endl;
    }
}

Mesh::~Mesh() {
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
    glDeleteVertexArrays(1, &m_vao);
    glDeleteProgram(m_shader_program);
}

void Mesh::load_mesh(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type, a, b, c, d;
        iss >> type;
        if (type == "v") {
            GLfloat x, y, z;
            iss >> x >> y >> z;
            m_vertices.push_back(Vertex{x, y, z});
        } else if (type == "f") {
            iss >> a >> b >> c;
            GLuint a_index, b_index, c_index;
            std::regex regex("(\\d+)/(\\d+)/(\\d+)");
            std::smatch match;
            if (std::regex_match(a, match, regex)) {
                a_index = std::stoi(match[1]) - 1;
            }
            if (std::regex_match(b, match, regex)) {
                b_index = std::stoi(match[1]) - 1;
            }
            if (std::regex_match(c, match, regex)) {
                c_index = std::stoi(match[1]) - 1;
            }
            m_indices.push_back(a_index);
            m_indices.push_back(b_index);
            m_indices.push_back(c_index);
            if (iss >> d) {
                GLuint d_index;
                if (std::regex_match(d, match, regex)) {
                    d_index = std::stoi(match[1]) - 1;
                }
                m_indices.push_back(a_index);
                m_indices.push_back(c_index);
                m_indices.push_back(d_index);
            }
        }
    }
}

Mesh::Mesh(const std::string& filename) {
    load_mesh(filename);
    setup_gl_objects();
    setup_shader();
}

void Mesh::setup_shader() {
    std::string vertex_shader_source = {
        "#version 460 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "gl_Position = vec4(aPos.x * 0.05, aPos.y * 0.05, aPos.z * 0.05, 1.0);\n"
        "}\n"
    };
    std::string fragment_shader_source = {
        "#version 460 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
        "}\n"
    };
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertex_shader_source_ptr = vertex_shader_source.c_str();
    glShaderSource(vertex_shader, 1, &vertex_shader_source_ptr, NULL);
    glCompileShader(vertex_shader);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragment_shader_source_ptr = fragment_shader_source.c_str();
    glShaderSource(fragment_shader, 1, &fragment_shader_source_ptr, NULL);
    glCompileShader(fragment_shader);
    m_shader_program = glCreateProgram();
    glAttachShader(m_shader_program, vertex_shader);
    glAttachShader(m_shader_program, fragment_shader);
    glLinkProgram(m_shader_program);
    glUseProgram(m_shader_program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

void Mesh::setup_gl_objects() {
    glCreateBuffers(1, &m_vbo);
    glCreateBuffers(1, &m_ebo);
    glNamedBufferStorage(m_vbo, m_vertices.size() * sizeof(m_vertices[0]), m_vertices.data(), 0);
    glNamedBufferStorage(m_ebo, m_indices.size() * sizeof(GLuint), m_indices.data(), 0);
    
    glCreateVertexArrays(1, &m_vao);
    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(GLfloat) * 3);
    glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(m_vao, 0, 0);
    glVertexArrayElementBuffer(m_vao, m_ebo);
    glEnableVertexArrayAttrib(m_vao, 0);

    glBindVertexArray(m_vao);
} 