#version 460 core
out vec4 outColor;

in vec3 fragNormal;  // from vertex shader
in vec3 fragPosition;
uniform vec3 kd;
uniform vec3 ks;
uniform vec3 light_intensity_ambient;
uniform vec3 light_intensity_diffuse;
uniform vec3 light_position;
uniform mat4 view_matrix;
void main() {
    vec3 n = normalize(fragNormal);
    vec3 light_position_view = (view_matrix * vec4(light_position, 1.0)).xyz;
    vec3 light_dir_view = normalize(light_position_view - fragPosition);
    vec3 half_vector = normalize(light_dir_view - fragPosition);
    float lambertian = max(dot(n, light_dir_view), 0.0);
    if (lambertian <= 0.0) {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    vec3 ambient = kd * light_intensity_ambient;
    vec3 diffuse = kd * lambertian * light_intensity_diffuse;
    float specular = pow(max(dot(n, half_vector), 0.0), 10.0);
    vec3 specular_color = ks * specular * light_intensity_diffuse;
    outColor = vec4(ambient + diffuse + specular_color, 1.0) * vec4(1.0, 1.0, 1.0, 1.0);
}