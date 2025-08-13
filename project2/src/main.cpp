#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Vector.h"
#include "Matrix4x4.h"
#include <array>
#include <algorithm>
#include "Mesh.h"
#include <memory>

void APIENTRY debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    std::cout << "Debug message: " << message << " " << source << " " << type
        << " " << id << " " << severity << " " << length << " " << userParam << std::endl;
}

struct Vertex {
    Vec3f position;
    Vec3f color;
};

class GlApp {
    private:
    GLFWwindow* m_window;
    int m_width, m_height;
    std::string m_title;
    GLuint m_shader_program;
    std::unique_ptr<Mesh> m_mesh;
    Matrix4x4 m_mvp;
    Matrix4x4 m_rotation;
    Matrix4x4 m_scale;
    Matrix4x4 m_translation;
    GLuint m_mvp_uniform_location;
    float m_angle;
    Matrix4x4 m_view;
    Matrix4x4 m_projection;
    float z_near = 0.01f;
    float z_far = 100.0f;
    float fov = 45.0f;
    float aspect_ratio = 800.0f / 600.0f;
    float m_camera_distance = 1.0f;
    float m_camera_yaw = 0.0f;
    float m_camera_pitch = 0.0f;

    bool m_left_mouse_pressed = false;
    bool m_right_mouse_pressed = false;

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
            app->m_camera_yaw += delta_x * 0.01f;
            app->m_camera_pitch += delta_y * 0.01f;
            app->m_camera_pitch = std::clamp(app->m_camera_pitch, -3.1416f, 3.1416f);
            app->m_camera_yaw = std::clamp(app->m_camera_yaw, -3.1416f, 3.1416f);
        }
        if (app->m_right_mouse_pressed) {
            app->m_camera_distance += delta_y * 0.001f;
            app->m_camera_distance = std::clamp(app->m_camera_distance, app->z_near, app->z_far);
        }
    }

    public:
    GlApp(int width, int height, std::string title) : m_width(width), m_height(height), m_title(title) {
        init_glfw(m_width, m_height, m_title);
        init_glew();
        init_shaders();
        init_gl_state();
        m_mesh = std::make_unique<Mesh>("teapot.obj", m_shader_program);
        m_mvp_uniform_location = glGetUniformLocation(m_shader_program, "mvp");
        m_rotation = Matrix4x4();
        m_scale = Matrix4x4();
        m_scale.set_scale(0.05f, 0.05f, 0.05f);
        m_angle = 0.0f;
        m_projection.set_perspective(fov, aspect_ratio, z_near, z_far);
        Vec3f center = m_mesh->get_bounding_box_center();
        m_translation.set_translation(-center.x, -center.y, -center.z);
        //m_projection.set_orthographic(-1.0f, 1.0f, -1.0f, 1.0f, z_near, z_far);
    }
    ~GlApp() {}

    std::string readFile(const char* path) {
        std::ifstream file(path);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    void run() {
        while (!glfwWindowShouldClose(m_window)) {
            render();
            glfwSwapBuffers(m_window);
            glfwPollEvents();
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
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

        m_window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
        glfwSetKeyCallback(m_window, key_callback);
        glfwSetWindowUserPointer(m_window, this);
        glfwSetMouseButtonCallback(m_window, mouse_button_callback);
        glfwSetCursorPosCallback(m_window, cursor_position_callback);
        if (!m_window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
        }

        glfwMakeContextCurrent(m_window);
    }

    void init_glew() {
        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW" << std::endl;
            glfwDestroyWindow(m_window);
            glfwTerminate();
            return;
        }

        std::cout << glGetString(GL_VERSION) << std::endl;
        std::cout << glGetString(GL_VENDOR) << std::endl;
        std::cout << glGetString(GL_RENDERER) << std::endl;
        std::cout << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        int flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        std::cout << "Context flags: " << flags << std::endl;

        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(debug_callback, NULL);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
            std::cout << "Debug context" << std::endl;
        } else {
            std::cout << "No debug context" << std::endl;
        }
    }

    void init_shaders() {
        m_shader_program = glCreateProgram();
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

        std::string vertexShaderSource = readFile("shaders/shader.vs");
        std::string fragmentShaderSource = readFile("shaders/shader.fs");

        const char* vertexShaderSrc = vertexShaderSource.c_str();
        const char* fragmentShaderSrc = fragmentShaderSource.c_str();
        glShaderSource(vertexShader, 1, &vertexShaderSrc, NULL);  
        glShaderSource(fragmentShader, 1, &fragmentShaderSrc, NULL);

        glCompileShader(vertexShader);
        GLint success;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE) {
            std::cerr << "Vertex shader compilation failed" << std::endl;
            return;
        }

        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE) {
            std::cerr << "Fragment shader compilation failed" << std::endl;
            return;
        }

        glAttachShader(m_shader_program, vertexShader);
        glAttachShader(m_shader_program, fragmentShader);
        glLinkProgram(m_shader_program);
        glGetProgramiv(m_shader_program, GL_LINK_STATUS, &success);
        if (success == GL_FALSE) {
            std::cerr << "Shader program linking failed" << std::endl;
            return;
        }

        glUseProgram(m_shader_program);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void init_gl_state() {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CW);
    }

    void render() {
        glfwGetFramebufferSize(m_window, &m_width, &m_height);
        glViewport(0, 0, m_width, m_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_rotation.set_rotation_y(m_angle);
        m_view.set_view(m_camera_distance, m_camera_yaw, m_camera_pitch);
        m_mvp = m_projection * m_view * m_rotation * m_scale * m_translation;
        m_mesh->draw(m_mvp);
    }
};

int main() {
    GlApp app(800, 600, "Hello World");
    app.run();
    return 0;
}