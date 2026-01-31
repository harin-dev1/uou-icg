#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cyMatrix.h>
#include <cyGL.h>
#include <cyTriMesh.h>
#include <iostream>
#include <vector>
#include <tuple>
#include <map>

struct Vertex {
    cy::Vec3f position;
    cy::Vec3f normal;
};

struct ColoredVertex {
    cy::Vec3f position;
    cy::Vec3f normal;
    cy::Vec3f color;
};

struct Light {
    cy::Vec3f position;
    cy::Vec3f intensity_ambient;
    cy::Vec3f intensity_diffuse;
    cy::Vec3f material_diffuse_color;
    cy::Vec3f material_specular_color;
};

struct MeshData {
    cyTriMesh mesh;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;
    cy::Matrix4f model;
};

struct ColoredMeshData {
    cyTriMesh mesh;
    std::vector<ColoredVertex> vertices;
    std::vector<uint32_t> indices;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;
    cy::Matrix4f model;
};

class GlApp {
private:
    GLFWwindow* m_window;
    int m_width, m_height;
    std::string m_title;

    cy::GLSLProgram m_shader_program;
    cy::GLSLProgram m_shadow_shader_program;
    GLuint m_rectangle_vao;
    GLuint m_rectangle_vbo;
    GLuint m_rectangle_ibo;

    int m_shadow_map_width = 4096;
    int m_shadow_map_height = 4096;
    cy::Matrix4f m_rectangle_model;
    cy::Matrix4f m_view;
    cy::Matrix4f m_projection;
    cy::Matrix4f m_mvp;
    cy::Matrix4f m_shadow_view;
    cy::Matrix4f m_shadow_projection;
    cy::Matrix4f m_shadow_mvp_texture_rectangle;
    bool m_left_mouse_pressed = false;
    bool m_right_mouse_pressed = false;
    float m_camera_distance = 2.0f;
    float m_camera_yaw = 0.0f;
    float m_camera_pitch = 0.3f;
    MeshData m_teapot;
    ColoredMeshData m_light_mesh;
    float m_floor_y_position = 0.0f;
    std::string m_teapot_obj_path;
    std::string m_light_obj_path;
    cy::GLSLProgram m_mesh_shader_program;
    cy::GLSLProgram m_light_shader_program;
    Light m_light;
    GLuint m_shadow_map_fbo;
    GLuint m_shadow_map_texture;
    bool m_ctrl_pressed = false;
    float m_light_pos_yaw = 0.0f;
    float m_light_pos_pitch = M_PI / 2.0f;
    float m_light_pos_distance = 1.0f;
public:
    GlApp(int width, int height, std::string title, std::string teapot_path, std::string light_path)
        : m_width(width), m_height(height), m_title(title), m_teapot_obj_path(teapot_path), m_light_obj_path(light_path) {
        init_glfw();
        init_glew();
        init_gl_state();
        init_shadow_map();
        init_shaders();
        init_rectangle();
        init_projection_matrices();
        init_meshes();
    }

    ~GlApp() {
        glDeleteVertexArrays(1, &m_rectangle_vao);
        glDeleteBuffers(1, &m_rectangle_vbo);
        glDeleteBuffers(1, &m_rectangle_ibo);
        glDeleteVertexArrays(1, &m_teapot.vao);
        glDeleteBuffers(1, &m_teapot.vbo);
        glDeleteBuffers(1, &m_teapot.ibo);
        glDeleteVertexArrays(1, &m_light_mesh.vao);
        glDeleteBuffers(1, &m_light_mesh.vbo);
        glDeleteBuffers(1, &m_light_mesh.ibo);
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
            if (app->m_left_mouse_pressed && !app->m_ctrl_pressed) {
                app->m_camera_yaw -= delta_x * 0.01f;
                app->m_camera_pitch += delta_y * 0.01f;
                app->m_camera_pitch = std::fmod(app->m_camera_pitch, 2.0f * M_PI);
                app->m_camera_yaw = std::fmod(app->m_camera_yaw, 2.0f * M_PI);
            }
            if (app->m_right_mouse_pressed && !app->m_ctrl_pressed) {
                app->m_camera_distance += delta_y * 0.001f;
            }
            if (app->m_ctrl_pressed && app->m_left_mouse_pressed) {
                app->m_light_pos_yaw -= delta_x * 0.01f;
                app->m_light_pos_pitch -= delta_y * 0.01f;
                app->m_light_pos_pitch = std::fmod(app->m_light_pos_pitch, 2.0f * M_PI);
                app->m_light_pos_yaw = std::fmod(app->m_light_pos_yaw, 2.0f * M_PI);
            }
            if (app->m_ctrl_pressed && app->m_right_mouse_pressed) {
                app->m_light_pos_distance += delta_y * 0.001f;
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
        m_light_shader_program.BuildFiles("shaders/light.vs", "shaders/light.fs");
        m_light.position = cy::Vec3f(0.0f, m_light_pos_distance, m_light_pos_distance);
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

    MeshData load_mesh(const std::string& path, bool compute_transform) {
        MeshData mesh_data;

        std::cout << "Loading mesh from " << path << std::endl;
        mesh_data.mesh.LoadFromFileObj(path.c_str(), false);

        std::map<std::tuple<int, int>, int> indices_map;
        for (uint32_t i = 0; i < mesh_data.mesh.NF(); i++) {
            for (uint32_t j = 0; j < 3; j++) {
                auto vertex_key = std::make_tuple(mesh_data.mesh.F(i).v[j], mesh_data.mesh.FN(i).v[j]);
                if (indices_map.find(vertex_key) == indices_map.end()) {
                    indices_map[vertex_key] = mesh_data.vertices.size();
                    mesh_data.vertices.push_back(Vertex{
                        mesh_data.mesh.V(mesh_data.mesh.F(i).v[j]),
                        mesh_data.mesh.VN(mesh_data.mesh.FN(i).v[j])
                    });
                }
                mesh_data.indices.push_back(indices_map[vertex_key]);
            }
        }

        glCreateVertexArrays(1, &mesh_data.vao);
        glCreateBuffers(1, &mesh_data.vbo);
        glCreateBuffers(1, &mesh_data.ibo);
        glNamedBufferStorage(mesh_data.vbo, mesh_data.vertices.size() * sizeof(Vertex), mesh_data.vertices.data(), 0);
        glNamedBufferStorage(mesh_data.ibo, mesh_data.indices.size() * sizeof(uint32_t), mesh_data.indices.data(), 0);
        glVertexArrayVertexBuffer(mesh_data.vao, 0, mesh_data.vbo, 0, sizeof(Vertex));
        glVertexArrayElementBuffer(mesh_data.vao, mesh_data.ibo);
        glVertexArrayAttribFormat(mesh_data.vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
        glVertexArrayAttribBinding(mesh_data.vao, 0, 0);
        glEnableVertexArrayAttrib(mesh_data.vao, 0);
        glVertexArrayAttribFormat(mesh_data.vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
        glVertexArrayAttribBinding(mesh_data.vao, 1, 0);
        glEnableVertexArrayAttrib(mesh_data.vao, 1);

        if (compute_transform) {
            mesh_data.mesh.ComputeBoundingBox();
            cy::Vec3f center = mesh_data.mesh.GetBoundMin() + (mesh_data.mesh.GetBoundMax() - mesh_data.mesh.GetBoundMin()) / 2.0f;
            cy::Vec3f size = mesh_data.mesh.GetBoundMax() - mesh_data.mesh.GetBoundMin();
            float max_size = std::max(size.x, std::max(size.y, size.z));
            cy::Matrix4f translation;
            translation.SetTranslation(-center);
            cy::Matrix4f scale;
            if (max_size > 1.0f) {
                scale.SetScale(1.0f / max_size);
            } else {
                scale.SetScale(1.0f);
            }
            cy::Matrix4f rotationX;
            rotationX.SetRotationX(-M_PI / 2.0f);
            mesh_data.model = rotationX * scale * translation;
        } else {
            mesh_data.model.SetIdentity();
        }

        return mesh_data;
    }

    ColoredMeshData load_light_mesh(const std::string& path) {
        ColoredMeshData mesh_data;

        std::cout << "Loading light mesh from " << path << std::endl;
        mesh_data.mesh.LoadFromFileObj(path.c_str(), true);  // Load with materials

        // Material colors from light.mtl
        cy::Vec3f frame_color(0.588f, 0.588f, 0.588f);  // Gray frame
        cy::Vec3f light_color(1.0f, 1.0f, 1.0f);        // White light/bulb

        // Build vertices with colors based on material
        std::map<std::tuple<int, int, int>, int> indices_map;  // pos, normal, material -> index
        for (uint32_t i = 0; i < mesh_data.mesh.NF(); i++) {
            // Determine material color for this face
            // cyTriMesh material index: 0 = Frame, 1 = Light (based on MTL order)
            cy::Vec3f color = frame_color;
            if (mesh_data.mesh.NM() > 0) {
                int mat_idx = mesh_data.mesh.GetMaterialIndex(i);
                if (mat_idx == 1) {  // Light material
                    color = light_color;
                }
            }

            for (uint32_t j = 0; j < 3; j++) {
                int pos_idx = mesh_data.mesh.F(i).v[j];
                int norm_idx = mesh_data.mesh.FN(i).v[j];
                int mat_key = (color == light_color) ? 1 : 0;

                auto vertex_key = std::make_tuple(pos_idx, norm_idx, mat_key);
                if (indices_map.find(vertex_key) == indices_map.end()) {
                    indices_map[vertex_key] = mesh_data.vertices.size();
                    mesh_data.vertices.push_back(ColoredVertex{
                        mesh_data.mesh.V(pos_idx),
                        mesh_data.mesh.VN(norm_idx),
                        color
                    });
                }
                mesh_data.indices.push_back(indices_map[vertex_key]);
            }
        }

        // Create VAO with color attribute
        glCreateVertexArrays(1, &mesh_data.vao);
        glCreateBuffers(1, &mesh_data.vbo);
        glCreateBuffers(1, &mesh_data.ibo);
        glNamedBufferStorage(mesh_data.vbo, mesh_data.vertices.size() * sizeof(ColoredVertex), mesh_data.vertices.data(), 0);
        glNamedBufferStorage(mesh_data.ibo, mesh_data.indices.size() * sizeof(uint32_t), mesh_data.indices.data(), 0);
        glVertexArrayVertexBuffer(mesh_data.vao, 0, mesh_data.vbo, 0, sizeof(ColoredVertex));
        glVertexArrayElementBuffer(mesh_data.vao, mesh_data.ibo);

        // Position attribute (location = 0)
        glVertexArrayAttribFormat(mesh_data.vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(ColoredVertex, position));
        glVertexArrayAttribBinding(mesh_data.vao, 0, 0);
        glEnableVertexArrayAttrib(mesh_data.vao, 0);

        // Normal attribute (location = 1)
        glVertexArrayAttribFormat(mesh_data.vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(ColoredVertex, normal));
        glVertexArrayAttribBinding(mesh_data.vao, 1, 0);
        glEnableVertexArrayAttrib(mesh_data.vao, 1);

        // Color attribute (location = 2)
        glVertexArrayAttribFormat(mesh_data.vao, 2, 3, GL_FLOAT, GL_FALSE, offsetof(ColoredVertex, color));
        glVertexArrayAttribBinding(mesh_data.vao, 2, 0);
        glEnableVertexArrayAttrib(mesh_data.vao, 2);

        mesh_data.model.SetIdentity();

        return mesh_data;
    }

    void init_meshes() {
        // Load teapot with computed transform
        m_teapot = load_mesh(m_teapot_obj_path, true);

        // Compute floor position based on teapot
        cy::Vec4f min_point = m_teapot.model * cy::Vec4f(m_teapot.mesh.GetBoundMin(), 1.0f);
        m_floor_y_position = min_point.y;
        m_rectangle_model.SetTranslation(cy::Vec3f(0.0f, m_floor_y_position, 0.0f));

        // Load light mesh with material colors
        m_light_mesh = load_light_mesh(m_light_obj_path);
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
        glCreateVertexArrays(1, &m_rectangle_vao);
        glCreateBuffers(1, &m_rectangle_vbo);
        glCreateBuffers(1, &m_rectangle_ibo);

        // Upload data
        glNamedBufferStorage(m_rectangle_vbo, sizeof(vertices), vertices, 0);
        glNamedBufferStorage(m_rectangle_ibo, sizeof(indices), indices, 0);

        // Setup vertex attributes
        glVertexArrayVertexBuffer(m_rectangle_vao, 0, m_rectangle_vbo, 0, sizeof(cy::Vec3f));
        glVertexArrayElementBuffer(m_rectangle_vao, m_rectangle_ibo);

        // Position attribute (location = 0)
        glVertexArrayAttribFormat(m_rectangle_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(m_rectangle_vao, 0, 0);
        glEnableVertexArrayAttrib(m_rectangle_vao, 0);
    }

    void init_projection_matrices() {
        m_projection.SetPerspective(45.0f, (float)m_width / (float)m_height, 0.1f, 100.0f);
        m_shadow_projection.SetPerspective(45.0f, (float)m_shadow_map_width / (float)m_shadow_map_height, 0.1f, 100.0f);
    }

    void render() {
        update_light();
        update_camera();
        render_shadow_map();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwGetFramebufferSize(m_window, &m_width, &m_height);
        glViewport(0, 0, m_width, m_height);

        // Draw rectangle (floor) with shadow
        m_shader_program.Bind();
        m_shader_program["mvp"] = m_mvp;
        glBindVertexArray(m_rectangle_vao);
        m_shader_program["shadowMVP"] = m_shadow_mvp_texture_rectangle;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_shadow_map_texture);
        m_shader_program["shadowMap"] = 0;
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        CY_GL_ERROR;

        // Draw teapot with lighting and shadow
        cy::Matrix4f teapot_mvp = m_projection * m_view * m_teapot.model;
        cy::Matrix4f teapot_mv = m_view * m_teapot.model;
        cy::Matrix4f scale_texture;
        scale_texture.SetScale(0.5f);
        cy::Matrix4f translation_texture;
        float bias = 0.01f;
        translation_texture.SetTranslation(cy::Vec3f(0.5f, 0.5f, 0.5f - bias));
        cy::Matrix4f teapot_shadow_mvp_texture = translation_texture * scale_texture * m_shadow_projection * m_shadow_view * m_teapot.model;

        m_mesh_shader_program.Bind();
        m_mesh_shader_program["mvp"] = teapot_mvp;
        m_mesh_shader_program["mv"] = teapot_mv;
        m_mesh_shader_program["mshadow"] = teapot_shadow_mvp_texture;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_shadow_map_texture);
        m_mesh_shader_program["shadowMap"] = 0;
        CY_GL_ERROR;
        // Set light struct members individually (cy::GLSLProgram doesn't support custom structs)
        cy::Vec4f light_pos_view = m_view * cy::Vec4f(m_light.position, 1.0f);
        m_mesh_shader_program["light.position_view"] = cy::Vec3f(light_pos_view.x, light_pos_view.y, light_pos_view.z);
        glBindVertexArray(m_teapot.vao);
        glDrawElements(GL_TRIANGLES, m_teapot.indices.size(), GL_UNSIGNED_INT, 0);

        // Draw light mesh at light position with material colors
        cy::Matrix4f scale_light;
        scale_light.SetScale(0.1f);  // Smaller scale
        cy::Matrix4f rotation_light;
        rotation_light.SetRotationX(-M_PI / 2.0f);  // Rotate to point downward
        cy::Matrix4f translation_light;
        translation_light.SetTranslation(m_light.position);
        m_light_mesh.model = translation_light * rotation_light * scale_light;
        cy::Matrix4f light_mesh_mvp = m_projection * m_view * m_light_mesh.model;
        m_light_shader_program.Bind();
        m_light_shader_program["mvp"] = light_mesh_mvp;
        glBindVertexArray(m_light_mesh.vao);
        glDrawElements(GL_TRIANGLES, m_light_mesh.indices.size(), GL_UNSIGNED_INT, 0);
    }

    void update_light() {
        m_light.position = cy::Vec3f(
            m_light_pos_distance * std::cos(m_light_pos_yaw) * std::cos(m_light_pos_pitch),
            m_light_pos_distance * std::sin(m_light_pos_pitch),
            m_light_pos_distance * std::sin(m_light_pos_yaw) * std::cos(m_light_pos_pitch)
        );
    }
    void update_camera() {
        float x = m_camera_distance * std::sin(m_camera_yaw) * std::cos(m_camera_pitch);
        float y = m_camera_distance * std::sin(m_camera_pitch);
        float z = m_camera_distance * std::cos(m_camera_yaw) * std::cos(m_camera_pitch);
        m_view.SetView(cy::Vec3f(x, y, z), cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
        m_mvp = m_projection * m_view * m_rectangle_model;
        m_shadow_view.SetView(m_light.position, cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
        cy::Matrix4f scale_texture;
        scale_texture.SetScale(0.5f);
        cy::Matrix4f translation_texture;
        float bias = 0.0001f;
        translation_texture.SetTranslation(cy::Vec3f(0.5f, 0.5f, 0.5f - bias));
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
        // Only render teapot to shadow map (teapot casts shadows, light mesh does not)
        cy::Matrix4f teapot_shadow_mvp = m_shadow_projection * m_shadow_view * m_teapot.model;
        m_shadow_shader_program["shadowMVP"] = teapot_shadow_mvp;
        glBindVertexArray(m_teapot.vao);
        glDrawElements(GL_TRIANGLES, m_teapot.indices.size(), GL_UNSIGNED_INT, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

int main() {
    GlApp app(800, 600, "Shadow Mapping", "models/teapot.obj", "models/light/light.obj");
    app.run();
    return 0;
}

