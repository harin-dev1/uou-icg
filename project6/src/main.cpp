#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include "cyTriMesh.h"
#include "cyMatrix.h"
#include "cyGL.h"
#include "lodepng.h"
#include <vector>

struct Vertex {
    cy::Vec3f position;
    cy::Vec3f normal;
};

class GlApp {
private:
    GLFWwindow* m_window;
    int m_width, m_height;
    std::string m_title;
    GLuint m_cubemap_texture;
    cyTriMesh m_cubemap_mesh;
    GLuint m_cubemap_vao;
    GLuint m_cubemap_vbo;
    GLuint m_cubemap_ibo;
    cy::GLSLProgram m_cubemap_shader_program;

    GLuint m_model_vao;
    GLuint m_model_vbo;
    GLuint m_model_ibo;
    cyTriMesh m_model_mesh;
    cy::GLSLProgram m_model_shader_program;

    cy::Matrix4f m_model_matrix;
    cy::Matrix4f m_view;
    cy::Matrix4f m_projection;
    cy::Matrix4f m_mvp;
    float m_camera_distance = 2.0f;
    float m_camera_yaw = 0.0f;
    float m_camera_pitch = 0.0f;
    bool m_left_mouse_pressed = false;
    public:
    GlApp(int width, int height, std::string title) : m_width(width), m_height(height), m_title(title) {
        init_glfw(m_width, m_height, m_title);
        init_glew();
        init_gl_state();
        init_cubemap_texture();
        init_cubemap_mesh();
        m_view = cy::Matrix4f::Identity();
        m_projection = cy::Matrix4f::Identity();
        m_projection.SetPerspective(45.0f, (float)m_width / (float)m_height, 0.1f, 100.0f);
        m_cubemap_shader_program.BuildFiles("shaders/cubemap.vs", "shaders/cubemap.fs");

        init_model_mesh();
        m_model_shader_program.BuildFiles("shaders/model.vs", "shaders/model.fs");
    }
    ~GlApp() {}

    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
        GlApp* app = (GlApp*)glfwGetWindowUserPointer(window);
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            app->m_left_mouse_pressed = (action == GLFW_PRESS);
        }
    }
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
        GlApp* app = (GlApp*)glfwGetWindowUserPointer(window);
        static double last_xpos = xpos;
        static double last_ypos = ypos;
        double delta_x = xpos - last_xpos;
        double delta_y = ypos - last_ypos;
        last_xpos = xpos;
        last_ypos = ypos;
        if (app->m_left_mouse_pressed) {
            app->m_camera_yaw += delta_x * 0.01f;
            app->m_camera_pitch += delta_y * 0.01f;
            app->m_camera_pitch = std::fmod(app->m_camera_pitch, 2.0f * M_PI);
            app->m_camera_yaw = std::fmod(app->m_camera_yaw, 2.0f * M_PI);
        }
    }
    void run() {
        while (!glfwWindowShouldClose(m_window)) {
            render();
            glfwSwapBuffers(m_window);
            glfwPollEvents();
        }
    }
    void render() {
        float x = m_camera_distance * std::sin(m_camera_yaw) * std::cos(m_camera_pitch);
        float y = m_camera_distance * std::sin(m_camera_pitch);
        float z = m_camera_distance * std::cos(m_camera_yaw) * std::cos(m_camera_pitch);
        m_view.SetView(cy::Vec3f(x, y, z), cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
        m_mvp = m_projection * cy::Matrix4f(cy::Matrix3f(m_view));
        glfwGetFramebufferSize(m_window, &m_width, &m_height);
        glViewport(0, 0, m_width, m_height);
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthMask(GL_FALSE);
        //glDepthFunc(GL_LEQUAL);
        m_cubemap_shader_program.Bind();
        m_cubemap_shader_program["mvp"] = m_mvp;
        m_cubemap_shader_program["cubemap"] = 0;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap_texture);
        glBindVertexArray(m_cubemap_vao);
        glDrawElements(GL_TRIANGLES, m_cubemap_mesh.NF() * 3, GL_UNSIGNED_INT, 0);
        glDepthMask(GL_TRUE);
        m_model_shader_program.Bind();  
        m_model_shader_program["model"] = m_model_matrix;
        m_model_shader_program["view"] = m_view;
        m_model_shader_program["projection"] = m_projection;
        m_model_shader_program["cameraPos"] = cy::Vec3f(x, y, z);
        m_model_shader_program["skybox"] = 0;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap_texture);
        glBindVertexArray(m_model_vao);
        glDrawElements(GL_TRIANGLES, m_model_mesh.NF() * 3, GL_UNSIGNED_INT, 0);
    }
    void init_glfw(int width, int height, std::string title) {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        m_window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
        glfwMakeContextCurrent(m_window);
        glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
        });
        glfwSetWindowUserPointer(m_window, this);
        glfwSetMouseButtonCallback(m_window, mouse_button_callback);
        glfwSetCursorPosCallback(m_window, cursor_position_callback);
    }
    void init_glew() {
        glewInit();
    }
    void init_gl_state() {
        glEnable(GL_DEPTH_TEST);
    }
    void load_cubemap_face(const std::string& face_path, GLenum face) {
        std::vector<unsigned char> data;
        unsigned width, height;
        unsigned error = lodepng::decode(data, width, height, face_path.c_str());
        if (error) {
            std::cerr << "Failed to load cubemap face: " << face_path << std::endl;
            return;
        }
        glTexImage2D(face, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    }
    void init_cubemap_texture() {
        glGenTextures(1, &m_cubemap_texture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap_texture);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        load_cubemap_face("cubemap/cubemap_posx.png", GL_TEXTURE_CUBE_MAP_POSITIVE_X);
        load_cubemap_face("cubemap/cubemap_negx.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
        load_cubemap_face("cubemap/cubemap_posy.png", GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
        load_cubemap_face("cubemap/cubemap_negy.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
        load_cubemap_face("cubemap/cubemap_posz.png", GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
        load_cubemap_face("cubemap/cubemap_negz.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
    }
    void load_mesh(const std::string& obj_path, float scale_factor, 
                   cyTriMesh& mesh, GLuint& vao, GLuint& vbo, GLuint& ibo) {
        mesh.LoadFromFileObj(obj_path.c_str());
        std::vector<cy::Vec3f> vertices;
        std::vector<GLuint> indices;
        for (uint32_t i = 0; i < mesh.NV(); i++) {
            vertices.push_back(mesh.V(i) * scale_factor);
        }
        for (uint32_t i = 0; i < mesh.NF(); i++) {
            for (uint32_t j = 0; j < 3; j++) {
                indices.push_back(mesh.F(i).v[j]);
            }
        }
        glCreateVertexArrays(1, &vao);
        glCreateBuffers(1, &vbo);
        glCreateBuffers(1, &ibo);
        glNamedBufferStorage(vbo, vertices.size() * sizeof(cy::Vec3f), vertices.data(), 0);
        glNamedBufferStorage(ibo, indices.size() * sizeof(GLuint), indices.data(), 0);
        glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(cy::Vec3f));
        glVertexArrayElementBuffer(vao, ibo);
        glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(vao, 0, 0);
        glEnableVertexArrayAttrib(vao, 0);
    }

    void load_mesh_with_normals(const std::string& obj_path, 
                                cyTriMesh& mesh, GLuint& vao, GLuint& vbo, GLuint& ibo) {
        mesh.LoadFromFileObj(obj_path.c_str());
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;
        std::map<std::tuple<int, int>, int> indices_map;
        for (uint32_t i = 0; i < mesh.NF(); i++) {
            for (uint32_t j = 0; j < 3; j++) {
                auto vertex_key = std::make_tuple(mesh.F(i).v[j], mesh.FN(i).v[j]);
                if (indices_map.find(vertex_key) == indices_map.end()) {
                    indices_map[vertex_key] = vertices.size();
                    vertices.push_back(Vertex{mesh.V(mesh.F(i).v[j]), mesh.VN(mesh.FN(i).v[j])});
                }
                indices.push_back(indices_map[vertex_key]);
            }
        }
        glCreateVertexArrays(1, &vao);
        glCreateBuffers(1, &vbo);
        glCreateBuffers(1, &ibo);
        glNamedBufferStorage(vbo, vertices.size() * sizeof(Vertex), vertices.data(), 0);
        glNamedBufferStorage(ibo, indices.size() * sizeof(GLuint), indices.data(), 0);
        glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));
        glVertexArrayElementBuffer(vao, ibo);
        glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
        glVertexArrayAttribBinding(vao, 0, 0);
        glEnableVertexArrayAttrib(vao, 0);
        glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
        glVertexArrayAttribBinding(vao, 1, 0);
        glEnableVertexArrayAttrib(vao, 1);
    }

    void init_cubemap_mesh() {
        load_mesh("cube/cube.obj", 1.0f, m_cubemap_mesh, m_cubemap_vao, m_cubemap_vbo, m_cubemap_ibo);
    }

    void init_model_mesh() {
        load_mesh_with_normals("models/teapot.obj", m_model_mesh, m_model_vao, m_model_vbo, m_model_ibo);
        m_model_matrix = cy::Matrix4f::Identity();
        m_model_mesh.ComputeBoundingBox();
        cy::Vec3f center = m_model_mesh.GetBoundMin() + (m_model_mesh.GetBoundMax() - m_model_mesh.GetBoundMin()) / 2.0f;
        cy::Vec3f size = m_model_mesh.GetBoundMax() - m_model_mesh.GetBoundMin();
        float max_size = std::max(size.x, std::max(size.y, size.z));
        cy::Matrix4f translation;
        translation.SetTranslation(-center);
        cy::Matrix4f scale;
        if (max_size > 0.0f) {
            //std::cout << "scale factor: " << 1.0f / max_size << std::endl;
            //scale.SetScale(0.205f / max_size);
            scale.SetScale(1.0f / max_size);
        } else {
            scale.SetScale(1.0f);
        }
        m_model_matrix = cy::Matrix4f::RotationX(-M_PI / 2.0f) * scale * translation;
    }
};

int main(int argc, char** argv) {
    GlApp app(1920, 1080, "Project 6");
    app.run();
    return 0;
}