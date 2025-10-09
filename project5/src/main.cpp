#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include "cyTriMesh.h"
#include "cyMatrix.h"
#include "cyGL.h"
#include <map>
#include <tuple>
#include "lodepng.h"
#include <vector>
struct Vertex {
    cy::Vec3f position;
    cy::Vec2f tex_coord;
};

class GlApp {
private:
    GLFWwindow* m_window;
    int m_width, m_height;
    std::string m_title;
    cyTriMesh m_mesh;
    std::vector<GLuint> m_mesh_vaos;
    std::vector<GLuint> m_mesh_vbos;
    std::vector<GLuint> m_mesh_ibos;
    std::vector<size_t> m_mesh_indices_sizes;
    cy::GLSLProgram m_mesh_shader_program;
    cy::Matrix4f m_mesh_model;
    cy::Matrix4f m_mesh_view;
    cy::Matrix4f m_mesh_projection;
    cy::Matrix4f m_mesh_mvp;
    bool m_left_mouse_pressed = false;
    bool m_right_mouse_pressed = false;
    float m_mesh_camera_distance = 1.0f;
    float m_mesh_camera_yaw = 0.0f;
    float m_mesh_camera_pitch = 0.0f;

    std::vector<GLuint> m_mesh_textures_kd;
    std::string m_model_obj_path;

    GLuint m_fbo;
    GLuint m_rbo_depth;
    GLuint m_texture_color;

    bool m_ctrl_pressed = false;
    float z_near = 0.01f;
    float z_far = 100.0f;

    GLuint m_square_vao;
    GLuint m_square_vbo;
    GLuint m_square_ibo;
    cy::GLSLProgram m_square_shader_program;
    cy::Matrix4f m_square_view;
    bool m_left_alt_pressed = false;
    bool m_right_alt_pressed = false;
    float m_camera_distance = 1.0f;
    float m_camera_yaw = 0.0f;
    float m_camera_pitch = 0.0f;

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        GlApp* app = (GlApp*)glfwGetWindowUserPointer(window);
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS) {
            app->m_ctrl_pressed = true;
        }
        if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE) {
            app->m_ctrl_pressed = false;
        }

        if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS) {
            app->m_left_alt_pressed = true;
        }
        if (key == GLFW_KEY_LEFT_ALT && action == GLFW_RELEASE) {
            app->m_left_alt_pressed = false;
        }
        if (key == GLFW_KEY_RIGHT_ALT && action == GLFW_PRESS) {
            app->m_right_alt_pressed = true;
        }
        if (key == GLFW_KEY_RIGHT_ALT && action == GLFW_RELEASE) {
            app->m_right_alt_pressed = false;
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
        if (app->m_left_mouse_pressed && !app->m_ctrl_pressed) {
            app->m_mesh_camera_yaw -= delta_x * 0.01f;
            app->m_mesh_camera_pitch += delta_y * 0.01f;
            app->m_mesh_camera_pitch = std::fmod(app->m_mesh_camera_pitch, 2.0f * M_PI);
            app->m_mesh_camera_yaw = std::fmod(app->m_mesh_camera_yaw, 2.0f * M_PI);
        }
        if (app->m_right_mouse_pressed && !app->m_ctrl_pressed) {
            app->m_mesh_camera_distance += delta_y * 0.001f;
            app->m_mesh_camera_distance = std::clamp(app->m_mesh_camera_distance, app->z_near, app->z_far);
        }

        if (app->m_left_alt_pressed && app->m_left_mouse_pressed) {
            app->m_camera_yaw -= delta_x * 0.01f;
            app->m_camera_pitch += delta_y * 0.01f;
            app->m_camera_pitch = std::fmod(app->m_camera_pitch, 2.0f * M_PI);
            app->m_camera_yaw = std::fmod(app->m_camera_yaw, 2.0f * M_PI);
        }
        if (app->m_left_alt_pressed && app->m_right_mouse_pressed) {
            app->m_camera_distance += delta_y * 0.001f;
            app->m_camera_distance = std::clamp(app->m_camera_distance, app->z_near, app->z_far);
        }
    }

    void init_fbo() {
        float max_anisotropy = 0.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &max_anisotropy);
        glCreateFramebuffers(1, &m_fbo);
        glCreateRenderbuffers(1, &m_rbo_depth);
        glCreateTextures(GL_TEXTURE_2D, 1, &m_texture_color);
        glTextureStorage2D(m_texture_color, 1, GL_RGBA8, 800, 600);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, max_anisotropy);
        glTextureParameteri(m_texture_color, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture_color, 0);
        glNamedRenderbufferStorage(m_rbo_depth, GL_DEPTH_COMPONENT, 800, 600);
        glNamedFramebufferRenderbuffer(m_fbo, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo_depth);
        CY_GL_ERROR;
        GLenum draw_buffers[] = {GL_COLOR_ATTACHMENT0};
        glNamedFramebufferDrawBuffers(m_fbo, 1, draw_buffers);
        CY_GL_ERROR;
    }
    void init_square() {
        m_square_shader_program.BuildFiles("shaders/shader.vs", "shaders/shader.fs");
        CY_GL_ERROR;
        std::vector<Vertex> square_vertices = {
            {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f}},
            {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f}},
            {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
            {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
        };
        std::vector<uint32_t> square_indices = {0, 1, 2, 0, 2, 3};
        glCreateVertexArrays(1, &m_square_vao);
        CY_GL_ERROR;
        glCreateBuffers(1, &m_square_vbo);
        glCreateBuffers(1, &m_square_ibo);
        
        glNamedBufferStorage(m_square_vbo, sizeof(square_vertices) * sizeof(Vertex), square_vertices.data(), 0);
        CY_GL_ERROR;
        glNamedBufferStorage(m_square_ibo, sizeof(square_indices) * sizeof(uint32_t), square_indices.data(), 0);
        CY_GL_ERROR;
        glVertexArrayVertexBuffer(m_square_vao, 0, m_square_vbo, 0, sizeof(Vertex));
        glVertexArrayElementBuffer(m_square_vao, m_square_ibo);
        CY_GL_ERROR;
        glVertexArrayAttribFormat(m_square_vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
        glVertexArrayAttribBinding(m_square_vao, 0, 0);
        glEnableVertexArrayAttrib(m_square_vao, 0);
        CY_GL_ERROR;
        glVertexArrayAttribFormat(m_square_vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, tex_coord));
        glVertexArrayAttribBinding(m_square_vao, 1, 0);
        glEnableVertexArrayAttrib(m_square_vao, 1);
        CY_GL_ERROR;
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

    // Helper function to create vertex data for a material
    void create_material_vertices(unsigned int material_id, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
        std::map<std::tuple<int, int>, int> indices_map;
        
        for (unsigned int j = 0; j < m_mesh.GetMaterialFaceCount(material_id); j++) {
            for (int k = 0; k < 3; k++) {
                int face_idx = m_mesh.GetMaterialFirstFace(material_id) + j;
                auto vertex_key = std::make_tuple(
                    m_mesh.F(face_idx).v[k],
                    m_mesh.FT(face_idx).v[k]
                );
                
                if (indices_map.find(vertex_key) == indices_map.end()) {
                    indices_map[vertex_key] = vertices.size();
                    vertices.push_back(Vertex{
                        m_mesh.V(m_mesh.F(face_idx).v[k]),
                        cy::Vec2f(m_mesh.VT(m_mesh.FT(face_idx).v[k]))
                    });
                }
                indices.push_back(indices_map[vertex_key]);
            }
        }
    }
    
    // Helper function to setup vertex attributes for a VAO
    void setup_vertex_attributes(GLuint vao) {
        // Position attribute (location 0)
        glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
        glVertexArrayAttribBinding(vao, 0, 0);
        glEnableVertexArrayAttrib(vao, 0);
        
        // Texture coordinate attribute (location 2)
        glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, tex_coord));
        glVertexArrayAttribBinding(vao, 1, 0);
        glEnableVertexArrayAttrib(vao, 1);
    }

    void init_mesh_vao() {
        std::cout << "Number of materials: " << m_mesh.NM() << std::endl;
        
        // Resize all vectors
        m_mesh_vaos.resize(m_mesh.NM());
        m_mesh_vbos.resize(m_mesh.NM());
        m_mesh_ibos.resize(m_mesh.NM());
        m_mesh_indices_sizes.resize(m_mesh.NM());
        
        for (unsigned int i = 0; i < m_mesh.NM(); i++) {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;
            
            // Create vertex data for this material
            create_material_vertices(i, vertices, indices);
            
            // Create OpenGL objects
            glCreateVertexArrays(1, &m_mesh_vaos[i]);
            glCreateBuffers(1, &m_mesh_vbos[i]);
            glCreateBuffers(1, &m_mesh_ibos[i]);

            // Upload data to buffers
            glNamedBufferStorage(m_mesh_vbos[i], vertices.size() * sizeof(Vertex), vertices.data(), 0);
            glNamedBufferStorage(m_mesh_ibos[i], indices.size() * sizeof(uint32_t), indices.data(), 0);

            // Bind buffers to VAO
            glVertexArrayVertexBuffer(m_mesh_vaos[i], 0, m_mesh_vbos[i], 0, sizeof(Vertex));
            glVertexArrayElementBuffer(m_mesh_vaos[i], m_mesh_ibos[i]);

            // Setup vertex attributes
            setup_vertex_attributes(m_mesh_vaos[i]);

            m_mesh_indices_sizes[i] = indices.size();
        }
    }
    void init_glew() {
        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW" << std::endl;
            return;
        }
    }

    void init_gl_state() {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
    }

    void render() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, 800, 600);
        render_mesh();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glfwGetFramebufferSize(m_window, &m_width, &m_height);
        glViewport(0, 0, m_width, m_height);
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_square();
    }
    void render_square() {
        m_square_view = cy::Matrix4f::Identity();
        float x = m_camera_distance * std::sin(m_camera_yaw) * std::cos(m_camera_pitch);
        float y = m_camera_distance * std::sin(m_camera_pitch);
        float z = m_camera_distance * std::cos(m_camera_yaw) * std::cos(m_camera_pitch);
        m_square_view.SetView(cy::Vec3f(x, y, z), cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
        m_square_shader_program.Bind();
        m_square_shader_program["mvp"] = m_square_view;
        m_square_shader_program["texture_color"] = 0;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture_color);
        CY_GL_ERROR;
        glBindVertexArray(m_square_vao);
        CY_GL_ERROR;
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        CY_GL_ERROR;
    }
    void render_mesh() {
        m_mesh_shader_program.Bind();
        m_mesh_view = cy::Matrix4f::Identity();
        float x = m_mesh_camera_distance * std::sin(m_mesh_camera_yaw) * std::cos(m_mesh_camera_pitch);
        float y = m_mesh_camera_distance * std::sin(m_mesh_camera_pitch);
        float z = m_mesh_camera_distance * std::cos(m_mesh_camera_yaw) * std::cos(m_mesh_camera_pitch);
        m_mesh_view.SetView(cy::Vec3f(x, y, z), cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
        m_mesh_mvp = m_mesh_projection * m_mesh_view * m_mesh_model;
        m_mesh_shader_program["mvp"] = m_mesh_mvp;
        for (unsigned int i = 0; i < m_mesh.NM(); i++) {
            m_mesh_shader_program["material.kd"] = cy::Vec3f(m_mesh.M(i).Kd[0], m_mesh.M(i).Kd[1], m_mesh.M(i).Kd[2]);

            // Bind textures using helper function
            bind_texture_if_available(m_mesh_textures_kd[i], m_mesh.M(i).map_Kd.data, 0, "has_texture_kd", "tex_kd");
            glBindVertexArray(m_mesh_vaos[i]);
            glDrawElements(GL_TRIANGLES, m_mesh_indices_sizes[i], GL_UNSIGNED_INT, 0);
        }
    }
    // Helper function to bind texture and set shader uniforms
    void bind_texture_if_available(GLuint texture_id, const char* texture_data, int texture_unit, 
                                   const char* has_texture_uniform, const char* texture_uniform) {
        if (texture_data != nullptr && texture_id != 0) {
            glActiveTexture(GL_TEXTURE0 + texture_unit);
            glBindTexture(GL_TEXTURE_2D, texture_id);
            m_mesh_shader_program[has_texture_uniform] = 1;
            m_mesh_shader_program[texture_uniform] = texture_unit;
        } else {
            m_mesh_shader_program[has_texture_uniform] = 0;
        }
    }

    // Helper function to load a single texture
    GLuint load_single_texture(const std::string& texture_path, const std::string& texture_type) {
        std::vector<unsigned char> data;
        unsigned width, height;
        unsigned error = lodepng::decode(data, width, height, texture_path.c_str());
        if (error) {
            std::cerr << "Failed to load " << texture_type << " texture: " << texture_path << std::endl;
            return 0;
        }
        
        std::cout << "Loaded " << texture_type << " texture: " << texture_path << std::endl;
        
        GLuint texture_id;
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        
        return texture_id;
    }

    void load_mesh_textures() {
        std::string model_directory = m_model_obj_path.substr(0, m_model_obj_path.find_last_of('/'));
        std::cout << "Model directory: " << model_directory << std::endl;
        
        // Initialize texture vectors
        m_mesh_textures_kd.resize(m_mesh.NM(), 0);
        
        std::cout << "Number of materials: " << m_mesh.NM() << std::endl;
        
        for (unsigned int i = 0; i < m_mesh.NM(); i++) {
            std::cout << "Processing material: " << m_mesh.M(i).name.data << std::endl;
            
            // Load diffuse texture (Kd)
            if (m_mesh.M(i).map_Kd.data != nullptr) {
                std::string texture_path = model_directory + "/" + std::string(m_mesh.M(i).map_Kd.data);
                m_mesh_textures_kd[i] = load_single_texture(texture_path, "diffuse");
            }
        }
    }
    void init_mesh() {
        m_mesh.LoadFromFileObj(m_model_obj_path.c_str());
        m_mesh.ComputeBoundingBox();
        cy::Vec3f center = m_mesh.GetBoundMin() + (m_mesh.GetBoundMax() - m_mesh.GetBoundMin()) / 2.0f;
        cy::Vec3f size = m_mesh.GetBoundMax() - m_mesh.GetBoundMin();
        float max_size = std::max(size.x, std::max(size.y, size.z));
        cy::Matrix4f translation;
        translation.SetTranslation(-center);
        cy::Matrix4f scale;
        if (max_size > 0.0f) {
            scale.SetScale(1.0f / max_size);
        } else {
            scale.SetScale(1.0f);
        }
        cy::Matrix4f rotation;
        rotation.SetRotation(cy::Vec3f(1.0f, 0.0f, 0.0f), -45.0f);
        m_mesh_model = rotation * scale * translation;
        m_mesh_shader_program.BuildFiles("shaders/mesh_shader.vs", "shaders/mesh_shader.fs");
        init_mesh_vao();
        m_mesh_view = cy::Matrix4f::Identity();
        m_mesh_projection = cy::Matrix4f::Identity();
        m_mesh_projection.SetPerspective(45.0f, (float)m_width / (float)m_height, z_near, z_far);
        load_mesh_textures();
    }
public:
    GlApp(int width, int height, std::string title, std::string model_obj_path) : m_width(width), m_height(height), m_title(title), m_model_obj_path(model_obj_path) {
        init_glfw(m_width, m_height, m_title);
        init_glew();
        init_gl_state();
        init_mesh();
        init_square();
        init_fbo();
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
int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <model_obj_path>" << std::endl;
        return 1;
    }
    std::string model_obj_path = argv[1];

    GlApp app(800, 600, "Project 4", model_obj_path);
    app.run();
    return 0;
}