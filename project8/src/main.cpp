#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cyMatrix.h>
#include <cyVector.h>
#include <cyGL.h>
#include <iostream>
#include <lodepng.h>

struct Vertex {
    cy::Vec3f position;
    cy::Vec2f tex_coord;
};

class GlApp {
    public:
    GlApp(int width, int height, std::string title, std::string normal_map_image_path, std::string displacement_map_image_path) : m_width(width), m_height(height), m_title(title), m_normal_map_image_path(normal_map_image_path), m_displacement_map_image_path(displacement_map_image_path) {
        init_glfw();
        init_glew();
        init_gl_state();
        init_quad_mesh();
        init_shaders();
        init_texture(normal_map_image_path, m_normal_map_texture);
        if (!displacement_map_image_path.empty()) {
            init_texture(displacement_map_image_path, m_displacement_map_texture);
        }
        init_projection_matrix();
        init_view_matrix();
        init_model_matrix();
        m_mvp = m_projection * m_view * m_model;
        m_mv_matrix = m_view * m_model;
    }
    ~GlApp() {
        glDeleteTextures(1, &m_normal_map_texture);
        if (!m_displacement_map_image_path.empty()) {
            glDeleteTextures(1, &m_displacement_map_texture);
        }
        glDeleteVertexArrays(1, &m_quad_vao);
        glDeleteBuffers(1, &m_quad_vbo);
        glDeleteBuffers(1, &m_quad_ibo);
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }
    void run() {
        while (!glfwWindowShouldClose(m_window)) {
            render();
            glfwSwapBuffers(m_window);
            glfwPollEvents();
        }
    }
    private:
    void init_glfw() {
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            exit(EXIT_FAILURE);
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), NULL, NULL);
        if (!m_window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
        glfwMakeContextCurrent(m_window);
        glfwSetWindowUserPointer(m_window, this);
        glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            GlApp* app = (GlApp*)glfwGetWindowUserPointer(window);
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
            if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
                app->m_render_wireframe = !app->m_render_wireframe;
            }
        });
    }
    void init_glew() {
        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW" << std::endl;
            glfwDestroyWindow(m_window);
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
    }
    void init_gl_state() {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CW);
        CY_GL_ERROR;
    }
    void render() {
        glfwGetFramebufferSize(m_window, &m_width, &m_height);
        glViewport(0, 0, m_width, m_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_quad();
        if (m_render_wireframe) {
            render_quad_gs();
        }
        CY_GL_ERROR;
    }
    void render_quad() {
        m_shader_program.Bind();
        m_shader_program["mvp"] = m_mvp;
        m_light_position_view = cy::Vec3f(m_view * cy::Vec4f(2.0f, 2.0f, 2.0f, 1.0f));
        m_shader_program["light_position_view"] = m_light_position_view;
        m_shader_program["mv_matrix"] = m_mv_matrix;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_normal_map_texture);
        m_shader_program.SetUniform("normalMap", 0);
        glBindVertexArray(m_quad_vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    void render_quad_gs() {
        m_shader_program_gs.Bind();
        m_shader_program_gs["mvp"] = m_mvp;
        glBindVertexArray(m_quad_vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    void init_quad_mesh() {
        std::vector<Vertex> vertices = {
            {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
            {{1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
            {{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}
        };
        std::vector<GLuint> indices = {0, 2, 1, 0, 3, 2};
        glCreateVertexArrays(1, &m_quad_vao);
        glCreateBuffers(1, &m_quad_vbo);
        glCreateBuffers(1, &m_quad_ibo);
        glNamedBufferStorage(m_quad_vbo, vertices.size() * sizeof(Vertex), vertices.data(), 0);
        glNamedBufferStorage(m_quad_ibo, indices.size() * sizeof(GLuint), indices.data(), 0);
        glVertexArrayVertexBuffer(m_quad_vao, 0, m_quad_vbo, 0, sizeof(Vertex));
        glVertexArrayElementBuffer(m_quad_vao, m_quad_ibo);
        glVertexArrayAttribFormat(m_quad_vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
        glVertexArrayAttribFormat(m_quad_vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, tex_coord));
        glVertexArrayAttribBinding(m_quad_vao, 0, 0);
        glEnableVertexArrayAttrib(m_quad_vao, 0);
        glVertexArrayAttribBinding(m_quad_vao, 1, 0);
        glEnableVertexArrayAttrib(m_quad_vao, 1);
        CY_GL_ERROR;
    }
    void init_shaders() {
        m_shader_program.BuildFiles("shaders/quad.vs", "shaders/quad.fs");
        m_shader_program_gs.BuildFiles("shaders/geometry.vs", "shaders/geometry.fs", "shaders/geometry.gs");
        CY_GL_ERROR;
    }
    void init_texture(std::string image_path, GLuint& texture) {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        std::vector<unsigned char> image;
        unsigned width, height;
        unsigned error = lodepng::decode(image, width, height, image_path.c_str());
        if (error) {
            std::cerr << "Failed to load normal map texture: " << image_path << std::endl;
            return;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        CY_GL_ERROR;
    }
    void init_projection_matrix() {
        m_projection.SetPerspective(45.0f, (float)m_width / (float)m_height, 0.01f, 100.0f);
    }
    void init_view_matrix() {
        m_view.SetView(cy::Vec3f(0.0f, 0.0f, 2.0f), cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
    }
    void init_model_matrix() {
        m_model.SetIdentity();
    }

    GLFWwindow* m_window;
    int m_width, m_height;
    std::string m_title;
    GLuint m_quad_vao;
    GLuint m_quad_vbo;
    GLuint m_quad_ibo;
    cy::GLSLProgram m_shader_program;
    cy::GLSLProgram m_shader_program_gs;
    GLuint m_normal_map_texture;
    std::string m_normal_map_image_path;
    std::string m_displacement_map_image_path;
    GLuint m_displacement_map_texture;
    cy::Matrix4f m_projection;
    cy::Matrix4f m_view;
    cy::Matrix4f m_model;
    cy::Matrix4f m_mvp;
    cy::Matrix4f m_mv_matrix;
    cy::Vec3f m_light_position_view;
    bool m_render_wireframe = false;
};

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <image_path>" << std::endl;
        return 1;
    }
    std::string normal_map_image_path = argv[1];
    std::string displacement_map_image_path = "";
    if (argc == 3) {
        displacement_map_image_path = argv[2];
    }
    GlApp app(800, 600, "Tessellation", normal_map_image_path, displacement_map_image_path);
    app.run();
    return 0;
}