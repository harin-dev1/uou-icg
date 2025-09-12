#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include "cyTriMesh.h"
#include "cyMatrix.h"
#include "cyGL.h"
#include <map>
#include <tuple>

struct Vertex {
    cy::Vec3f position;
    cy::Vec3f normal;
};

class GlApp {
private:
    GLFWwindow* m_window;
    int m_width, m_height;
    std::string m_title;
    cyTriMesh m_mesh;
    std::vector<Vertex> m_vertices;
    std::vector<int> m_indices;
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ibo;
    cy::GLSLProgram m_shader_program;
    cy::Matrix4f m_model;
    cy::Matrix4f m_view;
    cy::Matrix4f m_projection;
    cy::Matrix4f m_mvp;
    bool m_left_mouse_pressed = false;
    bool m_right_mouse_pressed = false;
    float m_camera_distance = 1.0f;
    float m_camera_yaw = 0.0f;
    float m_camera_pitch = 0.0f;
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
       if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
        GlApp* app = (GlApp*)glfwGetWindowUserPointer(window);
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            app->m_left_mouse_pressed = (action == GLFW_PRESS);
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            app->m_right_mouse_pressed = (action == GLFW_PRESS);
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
            app->m_camera_yaw -= delta_x * 0.01f;
            app->m_camera_pitch += delta_y * 0.01f;
            app->m_camera_pitch = std::fmod(app->m_camera_pitch, 2.0f * M_PI);
            app->m_camera_yaw = std::fmod(app->m_camera_yaw, 2.0f * M_PI);
        }
        if (app->m_right_mouse_pressed) {
            app->m_camera_distance += delta_y * 0.001f;
        }
    }

    void init_glfw(int width, int height, std::string title) {
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return;
        }
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        m_window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
        if (!m_window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return;
        }
        glfwSetKeyCallback(m_window, key_callback);
        glfwSetMouseButtonCallback(m_window, mouse_button_callback);
        glfwSetCursorPosCallback(m_window, cursor_position_callback);
        glfwSetWindowUserPointer(m_window, this);
        glfwMakeContextCurrent(m_window);
    }

    void init_vao() {

        std::map<std::tuple<int, int, int>, int> indices;
        for (unsigned int i = 0; i < m_mesh.NF(); i++) {
            for (int j = 0; j < 3; j++) {
                if (indices.find(std::make_tuple(m_mesh.F(i).v[j], m_mesh.FN(i).v[j], m_mesh.FT(i).v[j])) == indices.end()) {
                    indices[std::make_tuple(m_mesh.F(i).v[j], m_mesh.FN(i).v[j], m_mesh.FT(i).v[j])] = m_vertices.size();
                    m_vertices.push_back(Vertex{m_mesh.V(m_mesh.F(i).v[j]), m_mesh.VN(m_mesh.FN(i).v[j])});
                    m_indices.push_back(indices[std::make_tuple(m_mesh.F(i).v[j], m_mesh.FN(i).v[j], m_mesh.FT(i).v[j])]);
                } else {
                    m_indices.push_back(indices[std::make_tuple(m_mesh.F(i).v[j], m_mesh.FN(i).v[j], m_mesh.FT(i).v[j])]);
                }
            }
        }
        glCreateVertexArrays(1, &m_vao);
        glCreateBuffers(1, &m_vbo);
        glCreateBuffers(1, &m_ibo);

        glNamedBufferStorage(m_vbo, m_vertices.size() * sizeof(Vertex), m_vertices.data(), 0);
        glNamedBufferStorage(m_ibo, m_indices.size() * sizeof(int), m_indices.data(), 0);


        glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(Vertex));
        glVertexArrayVertexBuffer(m_vao, 1, m_vbo, offsetof(Vertex, normal), sizeof(Vertex));
        glVertexArrayElementBuffer(m_vao, m_ibo);

        glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(m_vao, 0, 0);
        glEnableVertexArrayAttrib(m_vao, 0);

        glVertexArrayAttribFormat(m_vao, 1, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(m_vao, 1, 0);
        glEnableVertexArrayAttrib(m_vao, 1);
    }
    void init_glew() {
        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW" << std::endl;
            return;
        }
    }
    void init_gl_state() {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
    }
    void render() {
        m_shader_program.Bind();
        glBindVertexArray(m_vao);
        glfwGetFramebufferSize(m_window, &m_width, &m_height);
        glViewport(0, 0, m_width, m_height);
        glClearColor(0.f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_view = cy::Matrix4f::Identity();
        float x = m_camera_distance * std::sin(m_camera_yaw) * std::cos(m_camera_pitch);
        float y = m_camera_distance * std::sin(m_camera_pitch);
        float z = m_camera_distance * std::cos(m_camera_yaw) * std::cos(m_camera_pitch);
        m_view.SetView(cy::Vec3f(x, y, z), cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
        m_mvp = m_projection * m_view * m_model;
        m_shader_program["mvp"] = m_mvp;
        m_shader_program["mv"] = m_view * m_model;
        glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
    }
public:
    GlApp(int width, int height, std::string title) : m_width(width), m_height(height), m_title(title) {
        init_glfw(m_width, m_height, m_title);
        init_glew();
        m_mesh.LoadFromFileObj("teapot.obj");
        m_mesh.ComputeBoundingBox();
        cy::Vec3f center = m_mesh.GetBoundMin() + (m_mesh.GetBoundMax() - m_mesh.GetBoundMin()) / 2.0f;
        cy::Matrix4f translation;
        translation.SetTranslation(-center);
        cy::Matrix4f scale;
        scale.SetScale(0.05f);
        cy::Matrix4f rotation;
        rotation.SetRotation(cy::Vec3f(1.0f, 0.0f, 0.0f), -45.0f);
        m_model = rotation * scale * translation;
        m_shader_program.BuildFiles("shaders/shader.vs", "shaders/shader.fs");
        init_vao();
        init_gl_state();
        m_view = cy::Matrix4f::Identity();
        m_projection = cy::Matrix4f::Identity();
        m_projection.SetPerspective(45.0f, (float)m_width / (float)m_height, 0.01f, 100.0f);
    }
    ~GlApp() {}
    void run() {
        while (!glfwWindowShouldClose(m_window)) {
            render();
            glfwSwapBuffers(m_window);
            glfwPollEvents();
        }
    }
};
int main() {

    GlApp app(800, 600, "Project 4");
    app.run();
    return 0;
}