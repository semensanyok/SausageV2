#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_draw_parameters : require

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

uniform mat4 projection_view;
uniform vec3 view_pos;

#define MAX_LIGHTS 1000

struct Light {
    vec3 direction;
    vec3 position;
    float spot_max_angle;
    vec3 color;
    unsigned int light_type;
};

layout (std430, binding = 0) buffer Transforms
{
    mat4 models[];
};

layout (std430, binding = 2) buffer Lights
{
    int num_lights;
    Light lights[];
};

out vs_out {
    int draw_id_arb;
    vec2 uv;
    vec3 frag_pos;
    
    vec3 tangent_light_pos[MAX_LIGHTS];
    vec3 tangent_view_pos;
    vec3 tangent_frag_pos;
} Out;

out vec2 uv_out;

void main(void) {
  mat4 model = models[gl_DrawIDARB];
  Out.frag_pos = vec3(model * vec4(position, 1.0));
  gl_Position = projection_view * vec4(Out.frag_pos, 1.0);
  Out.draw_id_arb = gl_DrawIDARB;
  Out.uv = uv;
  vec3 T = normalize(vec3(model * vec4(tangent, 0.0)));
  vec3 B = normalize(vec3(model * vec4(bitangent, 0.0)));
  vec3 N = normalize(vec3(model * vec4(normal, 0.0)));
  mat3 TBN = mat3(T, B, N);
  mat3 TBN_tangent = transpose(TBN); // == faster inverse  

  Out.tangent_view_pos = normalize(TBN_tangent * view_pos);
  Out.tangent_frag_pos = normalize(TBN_tangent * Out.frag_pos);
  for (int i = 0; i < num_lights; i++) {
    Out.tangent_light_pos[i] = normalize(TBN_tangent * lights[i].position);
  }
}
