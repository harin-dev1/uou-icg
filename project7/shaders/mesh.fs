#version 460 core

out vec4 outColor;
in vec3 fragNormalView;
in vec3 fragPositionView;
in vec4 fragPositionLightView;
uniform sampler2DShadow shadowMap;
struct Light {
    vec3 position_view;
    vec3 intensity_ambient;
    vec3 intensity_diffuse;
    vec3 material_diffuse_color;
    vec3 material_specular_color;
};
uniform Light light;
void main() {
    vec3 n = normalize(fragNormalView);
    vec3 light_dir = normalize(light.position_view - fragPositionView);
    vec3 view_dir = normalize(-fragPositionView);
    vec3 half_vector = normalize(light_dir + view_dir);
    float lambertian = max(dot(n, light_dir), 0.0);
    vec3 ambient = light.intensity_ambient * light.material_diffuse_color;
    if (lambertian <= 0.0) {
        outColor = vec4(ambient, 1.0);
        return;
    }
    vec3 diffuse = light.intensity_diffuse * lambertian * light.material_diffuse_color;
    vec3 specular = light.intensity_diffuse * pow(max(dot(n, half_vector), 0.0), 10.0) * light.material_specular_color;
    outColor = vec4(ambient + diffuse + specular, 1.0);
    outColor *= textureProj(shadowMap, fragPositionLightView);
}