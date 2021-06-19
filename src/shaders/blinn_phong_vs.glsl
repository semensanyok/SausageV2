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

layout (std430, binding = 1) buffer TransformOffset
{
    unsigned int transform_offset[];
};

layout (std430, binding = 0) buffer Transforms
{
    mat4 transforms[];
};

out vs_out {
    int draw_id_arb;
    vec2 uv;
    vec3 frag_pos;
    mat3 TBN;
} Out;

void main(void) {
  mat4 transform = transforms[transform_offset[gl_DrawIDARB] + gl_InstanceID];
  Out.frag_pos = vec3(transform * vec4(position, 1.0));
  gl_Position = projection_view * vec4(Out.frag_pos, 1.0);
  Out.draw_id_arb = gl_DrawIDARB;
  Out.uv = uv;
  vec3 T = normalize(vec3(transform * vec4(tangent, 0.0)));
  vec3 B = normalize(vec3(transform * vec4(bitangent, 0.0)));
  vec3 N = normalize(vec3(transform * vec4(normal, 0.0)));
  Out.TBN = mat3(T, B, N);
}
