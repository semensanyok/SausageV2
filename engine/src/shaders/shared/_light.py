light = {
    "light_enums":
"""
const int DIFFUSE_TEX = 0;
const int NORMAL_TEX = 1;
const int SPECULAR_TEX = 2;

const int POINT_LIGHT = 0;
const int DIRECTIONAL_LIGHT = 1;
const int SPOT_LIGHT = 2;
""",
    "light_structs":
"""
struct Light {
    vec4 direction;
    vec4 position;
    vec4 color;
    vec4 specular;
    
    int type;
    float spot_inner_cone_cos;
    float spot_outer_cone_cos;
    float constant_attenuation;

    float linear_attenuation;
    float quadratic_attenuation;
    float padding0;
    float padding1;
};
"""
    ,
    "light_uniforms":
"""
layout (std430, binding = 2) buffer Lights
{
	int num_lights;
	float padding0;
    float padding1;
	float padding2;
    Light lights[];
};
""",
	"light_functions": 
"""
vec3 GetSpecular(in Light light, in vec3 view_dir, in vec3 light_dir, in vec3 mat_specular) {
    vec3 half_way_dir = normalize(view_dir + light_dir);
    float spec = pow(max(dot(view_dir, half_way_dir), 0.0), blinn_spec_pow);
    return spec * vec3(light.specular) * mat_specular;
}

void AddDirectionalLight(in Light light, in vec3 view_dir, in vec3 mat_normal, in vec3 mat_diffuse, in vec3 mat_specular, inout vec3 res) {
	vec3 light_dir = normalize(vec3(-light.direction));
    vec3 light_diffuse = vec3(light.color) * max(dot(mat_normal, light_dir), 0.0) * mat_diffuse;
    res += light_diffuse;
    res += GetSpecular(light, view_dir, light_dir, mat_specular);
};

void AddPointLight(in vec3 frag_pos, in Light light, in vec3 view_dir, in vec3 mat_normal, in vec3 mat_diffuse, in vec3 mat_specular, inout vec3 res) {
	vec3 light_dir = normalize(vec3(light.position) - frag_pos);
    vec3 light_diffuse = vec3(light.color) * max(dot(mat_normal, light_dir), 0.0) * mat_diffuse;
    float distance_to_light = length(vec3(light.position) - frag_pos);
	float attenuation = 1.0 / (light.constant_attenuation + light.linear_attenuation * distance_to_light + light.quadratic_attenuation * pow(distance_to_light, 2));    
       
    res += (light_diffuse + GetSpecular(light, view_dir, light_dir, mat_specular)) * attenuation;
};

void AddSpotLight(in vec3 frag_pos, in Light light, in vec3 view_dir, in vec3 mat_normal, in vec3 mat_diffuse, in vec3 mat_specular, inout vec3 res)
{
    vec3 light_dir = normalize(vec3(light.position) - frag_pos);
    vec3 light_diffuse = vec3(light.color) * max(dot(mat_normal, light_dir), 0.0) * mat_diffuse;
	
    float distance_to_light = length(vec3(light.position) - frag_pos);
	float attenuation = 1.0 / (light.constant_attenuation + light.linear_attenuation * distance_to_light + light.quadratic_attenuation * pow(distance_to_light, 2));    
    
    float theta = dot(light_dir, normalize(-vec3(light.direction))); 
    float epsilon = light.spot_inner_cone_cos - light.spot_outer_cone_cos;
    float intensity = clamp((theta - light.spot_outer_cone_cos) / epsilon, 0.0, 1.0);
    
    res += (light_diffuse + GetSpecular(light, view_dir, light_dir, mat_specular)) * attenuation * intensity;
};

void AddLightColor(in vec3 frag_pos, in vec3 mat_normal, inout vec3 res, in vec3 view_dir, in vec3 mat_diffuse, in vec3 mat_specular) {
    for (int i = 0; i < num_lights; i++) {
        Light light = lights[i];
		if (light.type == POINT_LIGHT) {
            AddPointLight(frag_pos, light, view_dir, mat_normal, mat_diffuse, mat_specular, res);
        }
        if (light.type == DIRECTIONAL_LIGHT) {
            AddDirectionalLight(light, view_dir, mat_normal, mat_diffuse, mat_specular, res);
        }
		if (light.type == SPOT_LIGHT) {
            AddSpotLight(frag_pos, light, view_dir, mat_normal, mat_diffuse, mat_specular, res);
        }
    }
};
"""
}