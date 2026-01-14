#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cyMatrix.h>
#include <cyGL.h>
#include <cyTriMesh.h>
#include <iostream>
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
    
    cy::GLSLProgram m_shader_program;
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ibo;
    
    cy::Matrix4f m_mesh_model;
    cy::Matrix4f m_rectangle_model;
    cy::Matrix4f m_view;
    cy::Matrix4f m_projection;
    cy::Matrix4f m_mvp;
    cy::Matrix4f m_mesh_mvp;
    bool m_left_mouse_pressed = false;
    bool m_right_mouse_pressed = false;
    float m_camera_distance = 1.0f;
    float m_camera_yaw = 0.0f;
    float m_camera_pitch = 0.0f;
    std::vector<Vertex> m_mesh_vertices;
    std::vector<uint32_t> m_mesh_indices;
    cyTriMesh m_mesh;
    GLuint m_mesh_vao;
    GLuint m_mesh_vbo;
    GLuint m_mesh_ibo;
    float m_floor_y_position = 0.0f;
    std::string m_mesh_obj_path;
    cy::GLSLProgram m_mesh_shader_program;
public:
    GlApp(int width, int height, std::string title, std::string mesh_obj_path) 
        : m_width(width), m_height(height), m_title(title), m_mesh_obj_path(mesh_obj_path) {
        init_glfw();
        init_glew();
        init_gl_state();
        init_shaders();
        init_rectangle();
        init_matrices();
        init_mesh();
    }
    
    ~GlApp() {
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
        glDeleteBuffers(1, &m_ibo);
        glDeleteVertexArrays(1, &m_mesh_vao);
        glDeleteBuffers(1, &m_mesh_vbo);
        glDeleteBuffers(1, &m_mesh_ibo);
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
        
        // Set up callbacks
        glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
        });

        glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
            GlApp* app = (GlApp*)glfwGetWindowUserPointer(window);
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                app->m_left_mouse_pressed = (action == GLFW_PRESS);
            }
            if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                app->m_right_mouse_pressed = (action == GLFW_PRESS);
            }
        });
        glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xpos, double ypos) {
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
        });
        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
            GlApp* app = (GlApp*)glfwGetWindowUserPointer(window);
            app->m_width = width;
            app->m_height = height;
            app->m_projection.SetPerspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);
            glViewport(0, 0, width, height);
        });
    }
    
    void init_glew() {
        GLenum err = glewInit();
        if (err != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
            exit(EXIT_FAILURE);
        }
        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    }
    
    void init_gl_state() {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
    }
    
    void init_shaders() {
        m_shader_program.BuildFiles("shaders/rectangle.vs", "shaders/rectangle.fs");
        m_mesh_shader_program.BuildFiles("shaders/mesh.vs", "shaders/mesh.fs");
    }
    
    void init_mesh() {
        std::cout << "Loading mesh from " << m_mesh_obj_path << std::endl;
        m_mesh.LoadFromFileObj(m_mesh_obj_path.c_str(), false);

        std::map<std::tuple<int, int>, int> indices_map;
        for (uint32_t i = 0; i < m_mesh.NF(); i++) {
            for (uint32_t j = 0; j < 3; j++) {
                auto vertex_key = std::make_tuple(m_mesh.F(i).v[j], m_mesh.FN(i).v[j]);
                if (indices_map.find(vertex_key) == indices_map.end()) {
                    indices_map[vertex_key] = m_mesh_vertices.size();
                    m_mesh_vertices.push_back(Vertex{m_mesh.V(m_mesh.F(i).v[j]), m_mesh.VN(m_mesh.FN(i).v[j])});
                }
                m_mesh_indices.push_back(indices_map[vertex_key]);
            }
        }
        
        glCreateVertexArrays(1, &m_mesh_vao);
        glCreateBuffers(1, &m_mesh_vbo);
        glCreateBuffers(1, &m_mesh_ibo);
        glNamedBufferStorage(m_mesh_vbo, m_mesh_vertices.size() * sizeof(Vertex), m_mesh_vertices.data(), 0);
        glNamedBufferStorage(m_mesh_ibo, m_mesh_indices.size() * sizeof(uint32_t), m_mesh_indices.data(), 0);
        glVertexArrayVertexBuffer(m_mesh_vao, 0, m_mesh_vbo, 0, sizeof(Vertex));
        glVertexArrayElementBuffer(m_mesh_vao, m_mesh_ibo);
        glVertexArrayAttribFormat(m_mesh_vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
        glVertexArrayAttribBinding(m_mesh_vao, 0, 0);
        glEnableVertexArrayAttrib(m_mesh_vao, 0);
        glVertexArrayAttribFormat(m_mesh_vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
        glVertexArrayAttribBinding(m_mesh_vao, 1, 0);
        glEnableVertexArrayAttrib(m_mesh_vao, 1);

        m_mesh.ComputeBoundingBox();
        cy::Vec3f center = m_mesh.GetBoundMin() + (m_mesh.GetBoundMax() - m_mesh.GetBoundMin()) / 2.0f;
        cy::Vec3f size = m_mesh.GetBoundMax() - m_mesh.GetBoundMin();
        float max_size = std::max(size.x, std::max(size.y, size.z));
        cy::Matrix4f translation;
        translation.SetTranslation(-center);
        cy::Matrix4f scale;
        if (max_size > 1.0f) {
            scale.SetScale(1.0f / max_size);
        } else {
            scale.SetScale(1.0f / 1.0f);
        }
        cy::Matrix4f rotationX;
        rotationX.SetRotationX(-M_PI / 2.0f);
        m_mesh_model = rotationX * scale * translation;
        cy::Vec4f min_point = m_mesh_model * cy::Vec4f(m_mesh.GetBoundMin(), 1.0f);
        m_floor_y_position = min_point.y;
    }
    void init_rectangle() {
        // Rectangle vertices in XZ plane (y=0)
        // Counter-clockwise winding order (when viewed from above)
        cy::Vec3f vertices[] = {
            {-1.0f, 0.0f, -1.0f},  // back-left
            { 1.0f, 0.0f, -1.0f},  // back-right
            { 1.0f, 0.0f,  1.0f},  // front-right
            {-1.0f, 0.0f,  1.0f}   // front-left
        };
        
        // Two triangles to form a rectangle
        uint32_t indices[] = {
            0, 2, 1,  // first triangle
            0, 3, 2   // second triangle
        };
        
        // Create and setup VAO, VBO, and IBO
        glCreateVertexArrays(1, &m_vao);
        glCreateBuffers(1, &m_vbo);
        glCreateBuffers(1, &m_ibo);
        
        // Upload data
        glNamedBufferStorage(m_vbo, sizeof(vertices), vertices, 0);
        glNamedBufferStorage(m_ibo, sizeof(indices), indices, 0);
        
        // Setup vertex attributes
        glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(cy::Vec3f));
        glVertexArrayElementBuffer(m_vao, m_ibo);
        
        // Position attribute (location = 0)
        glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(m_vao, 0, 0);
        glEnableVertexArrayAttrib(m_vao, 0);
    }
    
    void init_matrices() {
        // Model matrix - identity (rectangle at origin)
        m_mesh_model.SetIdentity();
        m_rectangle_model.SetIdentity();
        
        // View matrix - camera looking at origin from a distance
        m_view.SetView(
            cy::Vec3f(1.0f, 1.0f, 1.0f),  // camera position
            cy::Vec3f(0.0f, 0.0f, 0.0f),  // look at point
            cy::Vec3f(0.0f, 1.0f, 0.0f)   // up vector
        );
    
        // Projection matrix - perspective
        m_projection.SetPerspective(45.0f, (float)m_width / (float)m_height, 0.1f, 100.0f);
        
        // Combined MVP matrix
        m_mvp = m_projection * m_view * m_rectangle_model;
        m_mesh_mvp = m_projection * m_view * m_mesh_model;
    }
    
    void render() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwGetFramebufferSize(m_window, &m_width, &m_height);
        glViewport(0, 0, m_width, m_height);

        m_rectangle_model.SetTranslation(cy::Vec3f(0.0f, m_floor_y_position, 0.0f));
        update_camera();
        m_shader_program.Bind();
        m_shader_program["mvp"] = m_mvp;
        
        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        m_mesh_shader_program.Bind();
        m_mesh_shader_program["mvp"] = m_mesh_mvp;
        glBindVertexArray(m_mesh_vao);
        glDrawElements(GL_TRIANGLES, m_mesh_indices.size(), GL_UNSIGNED_INT, 0);
    }

    void update_camera() {
        float x = m_camera_distance * std::sin(m_camera_yaw) * std::cos(m_camera_pitch);
        float y = m_camera_distance * std::sin(m_camera_pitch);
        float z = m_camera_distance * std::cos(m_camera_yaw) * std::cos(m_camera_pitch);
        m_view.SetView(cy::Vec3f(x, y, z), cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
        m_mvp = m_projection * m_view * m_rectangle_model;
        m_mesh_mvp = m_projection * m_view * m_mesh_model;
    }
};

int main() {
    GlApp app(800, 600, "Shadow Mapping", "models/teapot.obj");
    app.run();
    return 0;
}

