#version 460 core

out vec4 outColor;
in vec2 fragTCoord;
uniform sampler2D normalMap;
uniform vec3 light_position_view;
in vec3 fragPos;
void main() {
    vec3 normal = texture(normalMap, fragTCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    vec3 light_dir = normalize(light_position_view - fragPos);
    float lambertian = max(dot(normal, light_dir), 0.0);
    vec3 view_dir = normalize(-fragPos);
    vec3 half_vector = normalize(light_dir + view_dir);
    float specular = pow(max(dot(normal, half_vector), 0.0), 50.0); 
    vec3 diffuse = lambertian * vec3(0.5, 0.5, 0.5);
    vec3 specular_color = specular * vec3(1.0, 1.0, 1.0);
    outColor = vec4(diffuse + specular_color, 1.0);
}