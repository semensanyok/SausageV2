#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_draw_parameters : require

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

layout (std430, binding = 0) buffer Transforms
{
    mat4 mvp[];
};

out vs_out {
    int draw_id_arb;
    vec2 uv;
} Out;

void main(void) {
  gl_Position = mvp[gl_DrawIDARB] * vec4(position, 1.0);
  Out.uv = uv;
  Out.draw_id_arb = gl_DrawIDARB;
}