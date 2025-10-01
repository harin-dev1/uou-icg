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
    cy::Vec3f normal;
    cy::Vec2f tex_coord;
};
struct Light {
    cy::Vec3f position;
};

class GlApp {
private:
    GLFWwindow* m_window;
    int m_width, m_height;
    std::string m_title;
    cyTriMesh m_mesh;
    std::vector<GLuint> m_vaos;
    std::vector<GLuint> m_vbos;
    std::vector<GLuint> m_ibos;
    std::vector<size_t> m_indices_sizes;
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
    Light m_light;
    std::vector<GLuint> m_textures_kd;
    std::vector<GLuint> m_textures_ks;
    std::vector<GLuint> m_textures_ka;
    std::string m_model_obj_path;
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

    // Helper function to create vertex data for a material
    void create_material_vertices(unsigned int material_id, std::vector<Vertex>& vertices, std::vector<int>& indices) {
        std::map<std::tuple<int, int, int>, int> indices_map;
        
        for (unsigned int j = 0; j < m_mesh.GetMaterialFaceCount(material_id); j++) {
            for (int k = 0; k < 3; k++) {
                int face_idx = m_mesh.GetMaterialFirstFace(material_id) + j;
                auto vertex_key = std::make_tuple(
                    m_mesh.F(face_idx).v[k], 
                    m_mesh.FN(face_idx).v[k],
                    m_mesh.FT(face_idx).v[k]
                );
                
                if (indices_map.find(vertex_key) == indices_map.end()) {
                    indices_map[vertex_key] = vertices.size();
                    vertices.push_back(Vertex{
                        m_mesh.V(m_mesh.F(face_idx).v[k]),
                        m_mesh.VN(m_mesh.FN(face_idx).v[k]),
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
        
        // Normal attribute (location 1)
        glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
        glVertexArrayAttribBinding(vao, 1, 0);
        glEnableVertexArrayAttrib(vao, 1);
        
        // Texture coordinate attribute (location 2)
        glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, tex_coord));
        glVertexArrayAttribBinding(vao, 2, 0);
        glEnableVertexArrayAttrib(vao, 2);
    }

    void init_vao() {
        std::cout << "Number of materials: " << m_mesh.NM() << std::endl;
        
        // Resize all vectors
        m_vaos.resize(m_mesh.NM());
        m_vbos.resize(m_mesh.NM());
        m_ibos.resize(m_mesh.NM());
        m_indices_sizes.resize(m_mesh.NM());
        
        for (unsigned int i = 0; i < m_mesh.NM(); i++) {
            std::vector<Vertex> vertices;
            std::vector<int> indices;
            
            // Create vertex data for this material
            create_material_vertices(i, vertices, indices);
            
            // Create OpenGL objects
            glCreateVertexArrays(1, &m_vaos[i]);
            glCreateBuffers(1, &m_vbos[i]);
            glCreateBuffers(1, &m_ibos[i]);

            // Upload data to buffers
            glNamedBufferStorage(m_vbos[i], vertices.size() * sizeof(Vertex), vertices.data(), 0);
            glNamedBufferStorage(m_ibos[i], indices.size() * sizeof(int), indices.data(), 0);

            // Bind buffers to VAO
            glVertexArrayVertexBuffer(m_vaos[i], 0, m_vbos[i], 0, sizeof(Vertex));
            glVertexArrayElementBuffer(m_vaos[i], m_ibos[i]);

            // Setup vertex attributes
            setup_vertex_attributes(m_vaos[i]);

            m_indices_sizes[i] = indices.size();
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
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        m_light.position = cy::Vec3f(0.0f, 0.0f, -5.0f);
    }
    void render() {
        m_shader_program.Bind();
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
        m_shader_program["mv_inv_transpose"] = (m_view * m_model).GetInverse().GetTranspose();
        m_shader_program["mv"] = m_view * m_model;
        m_shader_program["light_position"] = m_view * m_light.position;
        for (unsigned int i = 0; i < m_mesh.NM(); i++) {
            m_shader_program["material.kd"] = cy::Vec3f(m_mesh.M(i).Kd[0], m_mesh.M(i).Kd[1], m_mesh.M(i).Kd[2]);
            m_shader_program["material.ks"] = cy::Vec3f(m_mesh.M(i).Ks[0], m_mesh.M(i).Ks[1], m_mesh.M(i).Ks[2]);
            m_shader_program["material.ka"] = cy::Vec3f(m_mesh.M(i).Ka[0], m_mesh.M(i).Ka[1], m_mesh.M(i).Ka[2]);
            m_shader_program["material.shininess"] = m_mesh.M(i).Ns;
            // Bind textures using helper function
            bind_texture_if_available(m_textures_kd[i], m_mesh.M(i).map_Kd.data, 0, "has_texture_kd", "tex_kd");
            bind_texture_if_available(m_textures_ks[i], m_mesh.M(i).map_Ks.data, 1, "has_texture_ks", "tex_ks");
            bind_texture_if_available(m_textures_ka[i], m_mesh.M(i).map_Ka.data, 2, "has_texture_ka", "tex_ka");
            glBindVertexArray(m_vaos[i]);
            glDrawElements(GL_TRIANGLES, m_indices_sizes[i], GL_UNSIGNED_INT, 0);
        }
    }

    // Helper function to bind texture and set shader uniforms
    void bind_texture_if_available(GLuint texture_id, const char* texture_data, int texture_unit, 
                                   const char* has_texture_uniform, const char* texture_uniform) {
        if (texture_data != nullptr && texture_id != 0) {
            glActiveTexture(GL_TEXTURE0 + texture_unit);
            glBindTexture(GL_TEXTURE_2D, texture_id);
            m_shader_program[has_texture_uniform] = 1;
            m_shader_program[texture_uniform] = texture_unit;
        } else {
            m_shader_program[has_texture_uniform] = 0;
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

    void load_texture() {
        std::string model_directory = m_model_obj_path.substr(0, m_model_obj_path.find_last_of('/'));
        std::cout << "Model directory: " << model_directory << std::endl;
        
        // Initialize texture vectors
        m_textures_kd.resize(m_mesh.NM(), 0);
        m_textures_ks.resize(m_mesh.NM(), 0);
        m_textures_ka.resize(m_mesh.NM(), 0);
        
        std::cout << "Number of materials: " << m_mesh.NM() << std::endl;
        
        for (unsigned int i = 0; i < m_mesh.NM(); i++) {
            std::cout << "Processing material: " << m_mesh.M(i).name.data << std::endl;
            
            // Load diffuse texture (Kd)
            if (m_mesh.M(i).map_Kd.data != nullptr) {
                std::string texture_path = model_directory + "/" + std::string(m_mesh.M(i).map_Kd.data);
                m_textures_kd[i] = load_single_texture(texture_path, "diffuse");
            }
            
            // Load specular texture (Ks)
            if (m_mesh.M(i).map_Ks.data != nullptr) {
                std::string texture_path = model_directory + "/" + std::string(m_mesh.M(i).map_Ks.data);
                m_textures_ks[i] = load_single_texture(texture_path, "specular");
            }
            
            // Load ambient texture (Ka)
            if (m_mesh.M(i).map_Ka.data != nullptr) {
                std::string texture_path = model_directory + "/" + std::string(m_mesh.M(i).map_Ka.data);
                m_textures_ka[i] = load_single_texture(texture_path, "ambient");
            }
        }
    }
public:
    GlApp(int width, int height, std::string title, std::string model_obj_path) : m_width(width), m_height(height), m_title(title), m_model_obj_path(model_obj_path) {
        init_glfw(m_width, m_height, m_title);
        init_glew();
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
        m_model = rotation * scale * translation;
        m_shader_program.BuildFiles("shaders/shader.vs", "shaders/shader.fs");
        init_vao();
        init_gl_state();
        m_view = cy::Matrix4f::Identity();
        m_projection = cy::Matrix4f::Identity();
        m_projection.SetPerspective(45.0f, (float)m_width / (float)m_height, 0.01f, 100.0f);
        load_texture();
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