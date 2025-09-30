#version 460 core
in vec3 fragNormal;
in vec3 fragPosition;
in vec2 fragTexCoord;
out vec4 outColor;

uniform vec3 light_position;

uniform sampler2D tex_kd;
uniform sampler2D tex_ks;
uniform sampler2D tex_ka;
uniform int has_texture_kd;
uniform int has_texture_ks;
uniform int has_texture_ka;

struct Material {
    vec3 kd;
    vec3 ks;
    vec3 ka;
    float shininess;
};

uniform Material material;
void main() {    
    vec3 n = normalize(fragNormal);
    vec3 light_dir = normalize(light_position - fragPosition);
    vec3 view_dir = normalize(-fragPosition);
    vec3 half_vector = normalize(light_dir + view_dir);
    float lambertian = max(dot(n, light_dir), 0.0);
    float specular = pow(max(dot(n, half_vector), 0.0), material.shininess);
    vec3 ambient_color = material.ka;
    vec3 diffuse_color = material.kd;
    vec3 specular_color = material.ks;
    if (has_texture_ka == 1) {
        ambient_color = texture(tex_ka, fragTexCoord).rgb * material.ka;
    }
    if (has_texture_kd == 1) {
        diffuse_color = texture(tex_kd, fragTexCoord).rgb * material.kd;
    }
    if (has_texture_ks == 1) {
        specular_color = texture(tex_ks, fragTexCoord).rgb * material.ks;
    }
    outColor = vec4(ambient_color + diffuse_color + specular_color, 1.0);
    //outColor = texture(tex_kd, fragTexCoord);
}