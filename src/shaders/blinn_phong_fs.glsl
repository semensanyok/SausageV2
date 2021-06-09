#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require

#define MAX_LIGHTS 1000

struct Light {
    vec3 direction;
    vec3 position;
    float spot_max_angle;
    vec3 color;
    unsigned int light_type;
};

out vec4 color;

in vs_out {
    int draw_id_arb;
    vec2 uv;
    vec3 frag_pos;
    
    vec3 tangent_light_pos[MAX_LIGHTS];
    vec3 tangent_view_pos;
    vec3 tangent_frag_pos;
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


float ambient_const = 0.1f;
float blinn_spec_pow = 32.0f;

void AddLightColor(vec3 normal, inout vec3 res) {
    vec3 tangent_view_dir = normalize(In.tangent_view_pos - In.tangent_frag_pos);
  for (int i = 0; i < num_lights; i++) {
    vec3 light_dir = lights[i].position - In.frag_pos;
    vec3 light_color = lights[i].color;
    vec3 light_diffuse = max(dot(normal, light_dir), 0.0) * light_color;
    res += light_diffuse;

    vec3 tangent_light_dir = normalize(In.tangent_light_pos[i] - In.tangent_frag_pos);
    vec3 tangent_half_way_dir = normalize(tangent_view_dir + tangent_light_dir);
    float spec = pow(max(dot(tangent_half_way_dir, normal), 0.0), blinn_spec_pow);
    res += spec;
  }
}

void main(void) {
  vec3 diffuse = texture(textures[In.draw_id_arb], vec3(In.uv, 0)).rgb;
//  vec3 specular = texture(textures[In.draw_id_arb], vec3(In.uv,2)).rgb;
  vec3 normal = normalize(texture(textures[In.draw_id_arb], vec3(In.uv,1)).rgb * 2.0 - 1.0);
  
  vec3 res = diffuse * ambient_const;
  AddLightColor(normal, res);
  color = vec4(res, 1.0);
}
