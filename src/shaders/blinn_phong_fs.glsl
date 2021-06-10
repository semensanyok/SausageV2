#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require

#define MAX_LIGHTS 1000

uniform vec3 view_pos;

struct Light {
    vec3 direction;
    vec3 position;
    float spot_max_angle;
    vec3 color;
    unsigned int light_type;
};

out vec4 color;

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


float ambient_const = 0.1f;
float blinn_spec_pow = 32.0f;

void AddLightColor(vec3 normal, inout vec3 res) {
    vec3 view_dir = normalize(view_pos - In.frag_pos);
    for (int i = 0; i < num_lights; i++) {
      vec3 light_dir = normalize(lights[i].position - In.frag_pos);
      vec3 light_color = lights[i].color;
      vec3 light_diffuse = max(dot(normal, light_dir), 0.0) * light_color;
      res += light_diffuse;
      
      vec3 half_way_dir = normalize(view_dir + light_dir);
      float spec = pow(max(dot(half_way_dir, normal), 0.0), blinn_spec_pow);
      res += spec;
  }
}

void main(void) {
  vec3 diffuse = texture(textures[In.draw_id_arb], vec3(In.uv, 0)).rgb;
//  vec3 specular = texture(textures[In.draw_id_arb], vec3(In.uv,2)).rgb;
  vec3 normal = texture(textures[In.draw_id_arb], vec3(In.uv,1)).rgb * 2.0 - 1.0;
  normal = normalize(In.TBN * normal);
  vec3 res = diffuse * ambient_const;
  AddLightColor(normal, res);
  color = vec4(res, 1.0);
}
