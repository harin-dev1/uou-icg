#version 460 core

out vec4 outColor;
in vec3 Normal;
in vec3 Position;
uniform samplerCube skybox;
uniform vec3 cameraPos;
uniform vec3 light_position;
void main() {
    vec3 view_dir = normalize(cameraPos - Position);
    vec3 light_dir = normalize(light_position - Position);
    vec3 half_vector = normalize(light_dir + view_dir);
    vec3 normal = normalize(Normal);
    float lambertian = max(dot(normal, light_dir), 0.0);
    float specular_term = pow(max(dot(normal, half_vector), 0.0), 10.0);
    vec3 R = reflect(view_dir * -1.0, normal);
    vec3 skybox_color = texture(skybox, R).rgb;
    vec3 ambient = 0.1 * skybox_color;
    vec3 diffuse = lambertian * skybox_color;
    vec3 specular = specular_term * skybox_color;
    outColor = vec4(ambient + diffuse + specular, 1.0);
}