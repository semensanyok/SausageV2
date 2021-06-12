#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require

int POINT_LIGHT = 0;
int DIRECTIONAL_LIGHT = 1;
int SPOT_LIGHT = 2;

float OGRE_P_L_ATT_DIST_7L = 0.7f;
float OGRE_P_L_ATT_DIST_7Q = 1.8f;
float OGRE_P_L_ATT_DIST_13L = 0.35f;
float OGRE_P_L_ATT_DIST_13Q = 0.44f;
float OGRE_P_L_ATT_DIST_20L = 0.22f;
float OGRE_P_L_ATT_DIST_20Q = 0.20f;

uniform vec3 view_pos;

out vec4 color;

float ambient_const = 0.1f;
float blinn_spec_pow = 32.0f;

struct Light {
    vec3 direction;
    vec3 position;
    float spot_max_angle;
    vec3 color;
    vec3 specular;
    unsigned int type;
    float constant_attenuation;
    float linear_attenuation;
    float quadratic_attenuation;
};


in vs_out {
    flat int draw_id_arb;
    vec2 uv;
    vec3 frag_pos;
    mat3 TBN;
} In;

// Texture block
layout (std430, binding = 1) buffer TextureArray
{
    sampler2DArray textures[];
};

layout (std430, binding = 2) buffer Lights
{
    int num_lights;
    Light lights[];
};

Light test_lights[2];


void AddSpecular(in Light light, in vec3 view_dir, in vec3 light_dir, in vec3 mat_specular, inout vec3 res) {
    vec3 half_way_dir = normalize(view_dir + light_dir);
    float spec = pow(max(dot(view_dir, half_way_dir), 0.0), blinn_spec_pow);
    res += spec * light.specular * mat_specular;
}

void AddPointLight(in Light light, in vec3 view_dir, in vec3 mat_normal, in vec3 mat_diffuse, in vec3 mat_specular, inout vec3 res) {
	vec3 light_dir = normalize(light.direction - In.frag_pos);
    vec3 light_diffuse = light.color * max(dot(mat_normal, light_dir), 0.0) * mat_diffuse;
    float distance_to_light = length(light.position - In.frag_pos);
    float attenuation = 1.0 / (light.constant_attenuation + light.linear_attenuation * distance_to_light + light.quadratic_attenuation * pow(distance_to_light, 2));    
    
    AddSpecular(light, view_dir, light_dir, mat_specular, res);
}

void AddDirectionalLight(in Light light, in vec3 view_dir, in vec3 mat_normal, in vec3 mat_diffuse, in vec3 mat_specular, inout vec3 res) {
	vec3 light_dir = normalize(-light.direction);
    vec3 light_diffuse = light.color * max(dot(mat_normal, light_dir), 0.0) * mat_diffuse;
    res += light_diffuse;
    AddSpecular(light, view_dir, light_dir, mat_specular, res);
}

void AddLightColor(in vec3 mat_normal, inout vec3 res, in vec3 view_dir, in vec3 mat_diffuse, in vec3 mat_specular) {
    for (int i = 0; i < 2; i++) {
        Light light = test_lights[i];
        if (light.type == POINT_LIGHT) {
            AddPointLight(light, view_dir, mat_normal, mat_diffuse, mat_specular, res);
        }
        if (light.type == DIRECTIONAL_LIGHT) {
            AddDirectionalLight(light, view_dir, mat_normal, mat_diffuse, mat_specular, res);
        }
    }
}

void main(void) {
  test_lights = Light[2](
  Light(vec3(0,0,0), vec3(-5,5,5),0,vec3(100,100,100),vec3(100,100,100),DIRECTIONAL_LIGHT, 1, OGRE_P_L_ATT_DIST_7L, OGRE_P_L_ATT_DIST_7Q),
  Light(vec3(0,0,0), vec3(5,5,5),0,vec3(50,50,50),vec3(50,50,50),DIRECTIONAL_LIGHT,1,OGRE_P_L_ATT_DIST_7L, OGRE_P_L_ATT_DIST_7Q)
  );
  vec3 mat_diffuse = texture(textures[In.draw_id_arb], vec3(In.uv, 0)).rgb;
  vec3 mat_specular = texture(textures[In.draw_id_arb], vec3(In.uv,2)).rgb;
  vec3 mat_normal = texture(textures[In.draw_id_arb], vec3(In.uv,1)).rgb * 2.0 - 1.0;
  mat_normal = normalize(In.TBN * mat_normal);
  vec3 view_dir = normalize(view_pos - In.frag_pos);
  vec3 res = mat_diffuse * ambient_const;
  AddLightColor(mat_normal, res, view_dir, mat_diffuse, mat_specular);
  color = vec4(res, 1.0);
}
