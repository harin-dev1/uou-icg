#version 460 core
out vec4 outColor;

in vec3 fragNormal;  // from vertex shader
in vec3 fragPosition;
uniform vec3 kd;
uniform vec3 ks;
uniform vec3 light_intensity_ambient;
uniform vec3 light_intensity_diffuse;
uniform vec3 light_position;
void main() {
    vec3 n = normalize(fragNormal);
    vec3 light_dir = normalize(light_position - fragPosition);
    vec3 half_vector = normalize(light_dir - fragPosition);
    float lambertian = max(dot(n, light_dir), 0.0);
    vec3 ambient = kd * light_intensity_ambient;
    vec3 diffuse = kd * lambertian * light_intensity_diffuse;
    float specular = pow(max(dot(n, half_vector), 0.0), 100.0);
    vec3 specular_color = ks * specular * light_intensity_diffuse;
    outColor = vec4(ambient + diffuse + specular_color, 1.0) * vec4(1.0, 1.0, 1.0, 1.0);
}