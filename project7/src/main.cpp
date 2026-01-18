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

struct Light {
    cy::Vec3f position;
    cy::Vec3f intensity_ambient;
    cy::Vec3f intensity_diffuse;
    cy::Vec3f material_diffuse_color;
    cy::Vec3f material_specular_color;
};

class GlApp {
private:
    GLFWwindow* m_window;
    int m_width, m_height;
    std::string m_title;
    
    cy::GLSLProgram m_shader_program;
    cy::GLSLProgram m_shadow_shader_program;
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ibo;
    
    int m_shadow_map_width = 4096;
    int m_shadow_map_height = 4096;
    cy::Matrix4f m_mesh_model;
    cy::Matrix4f m_rectangle_model;
    cy::Matrix4f m_view;
    cy::Matrix4f m_projection;
    cy::Matrix4f m_mvp;
    cy::Matrix4f m_mesh_mvp;
    cy::Matrix4f m_mesh_mv;
    cy::Matrix4f m_shadow_mvp;
    cy::Matrix4f m_shadow_view;
    cy::Matrix4f m_shadow_projection;
    cy::Matrix4f m_shadow_mvp_texture;
    cy::Matrix4f m_shadow_mvp_texture_rectangle;
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
    Light m_light;
    GLuint m_shadow_map_fbo;
    GLuint m_shadow_map_texture;
public:
    GlApp(int width, int height, std::string title, std::string mesh_obj_path) 
        : m_width(width), m_height(height), m_title(title), m_mesh_obj_path(mesh_obj_path) {
        init_glfw();
        init_glew();
        init_gl_state();
        init_shadow_map();
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
        glDeleteFramebuffers(1, &m_shadow_map_fbo);
        glDeleteTextures(1, &m_shadow_map_texture);
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
        m_light.position = cy::Vec3f(0.0f, 5.0f, 5.0f);
        m_light.intensity_ambient = cy::Vec3f(0.1f, 0.1f, 0.1f);
        m_light.intensity_diffuse = cy::Vec3f(1.0f, 1.0f, 1.0f);
        m_light.material_diffuse_color = cy::Vec3f(1.0f, 0.0f, 0.0f);
        m_light.material_specular_color = cy::Vec3f(1.0f, 1.0f, 1.0f);
        m_mesh_shader_program["light.intensity_ambient"] = m_light.intensity_ambient;
        m_mesh_shader_program["light.intensity_diffuse"] = m_light.intensity_diffuse;
        m_mesh_shader_program["light.material_diffuse_color"] = m_light.material_diffuse_color;
        m_mesh_shader_program["light.material_specular_color"] = m_light.material_specular_color;
        m_shadow_shader_program.BuildFiles("shaders/shadow.vs", "shaders/shadow.fs");
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
        m_mesh_mv = m_view * m_mesh_model;

        m_shadow_view.SetView(m_light.position, cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
        m_shadow_projection.SetPerspective(45.0f, (float)m_shadow_map_width / (float)m_shadow_map_height, 0.1f, 100.0f);
        m_shadow_mvp = m_shadow_projection * m_shadow_view * m_mesh_model;
        cy::Matrix4f scale_texture;
        scale_texture.SetScale(0.5f);
        cy::Matrix4f translation_texture;
        translation_texture.SetTranslation(cy::Vec3f(0.5f, 0.5f, 0.5f));
        m_shadow_mvp_texture = translation_texture * scale_texture * m_shadow_projection * m_shadow_view * m_mesh_model;
        m_shadow_mvp_texture_rectangle = translation_texture * scale_texture * m_shadow_projection * m_shadow_view * m_rectangle_model;
    }
    
    void render() {
        update_camera();
        render_shadow_map();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwGetFramebufferSize(m_window, &m_width, &m_height);
        glViewport(0, 0, m_width, m_height);

        m_rectangle_model.SetTranslation(cy::Vec3f(0.0f, m_floor_y_position, 0.0f));
        m_shader_program.Bind();
        m_shader_program["mvp"] = m_mvp;
        
        glBindVertexArray(m_vao);
        m_shader_program["shadowMVP"] = m_shadow_mvp_texture_rectangle;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_shadow_map_texture);
        m_shader_program["shadowMap"] = 0;
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        CY_GL_ERROR;
        m_mesh_shader_program.Bind();
        m_mesh_shader_program["mvp"] = m_mesh_mvp;
        m_mesh_shader_program["mv"] = m_mesh_mv;
        m_mesh_shader_program["mshadow"] = m_shadow_mvp_texture;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_shadow_map_texture);
        m_mesh_shader_program["shadowMap"] = 0;
        CY_GL_ERROR;
        // Set light struct members individually (cy::GLSLProgram doesn't support custom structs)
        cy::Vec4f light_pos_view = m_view * cy::Vec4f(m_light.position, 1.0f);
        m_mesh_shader_program["light.position_view"] = cy::Vec3f(light_pos_view.x, light_pos_view.y, light_pos_view.z);
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
        m_mesh_mv = m_view * m_mesh_model;
        m_shadow_view.SetView(m_light.position, cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
        m_shadow_mvp = m_shadow_projection * m_shadow_view * m_mesh_model;
        cy::Matrix4f scale_texture;
        scale_texture.SetScale(0.5f);
        cy::Matrix4f translation_texture;
        float bias = 0.0001f;
        translation_texture.SetTranslation(cy::Vec3f(0.5f, 0.5f, 0.5f - bias));
        m_shadow_mvp_texture = translation_texture * scale_texture * m_shadow_projection * m_shadow_view * m_mesh_model;
        m_shadow_mvp_texture_rectangle = translation_texture * scale_texture * m_shadow_projection * m_shadow_view * m_rectangle_model;
    }

    void init_shadow_map() {
        glCreateFramebuffers(1, &m_shadow_map_fbo);
        glCreateTextures(GL_TEXTURE_2D, 1, &m_shadow_map_texture);
        glTextureStorage2D(m_shadow_map_texture, 1, GL_DEPTH_COMPONENT24, m_shadow_map_width, m_shadow_map_height);
        glTextureParameteri(m_shadow_map_texture, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTextureParameteri(m_shadow_map_texture, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glTextureParameteri(m_shadow_map_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_shadow_map_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(m_shadow_map_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTextureParameteri(m_shadow_map_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        cy::Vec4f border_color(1.0f, 1.0f, 1.0f, 1.0f);
        glTextureParameterfv(m_shadow_map_texture, GL_TEXTURE_BORDER_COLOR, &border_color.x);
        glNamedFramebufferTexture(m_shadow_map_fbo, GL_DEPTH_ATTACHMENT, m_shadow_map_texture, 0);
        glNamedFramebufferDrawBuffer(m_shadow_map_fbo, GL_NONE);
        glNamedFramebufferReadBuffer(m_shadow_map_fbo, GL_NONE);
        CY_GL_ERROR;
    }

    void render_shadow_map() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_shadow_map_fbo);
        glClear(GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, m_shadow_map_width, m_shadow_map_height);
        m_shadow_shader_program.Bind();
        m_shadow_shader_program["shadowMVP"] = m_shadow_mvp;
        glBindVertexArray(m_mesh_vao);
        glDrawElements(GL_TRIANGLES, m_mesh_indices.size(), GL_UNSIGNED_INT, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

int main() {
    GlApp app(800, 600, "Shadow Mapping", "models/teapot.obj");
    app.run();
    return 0;
}

