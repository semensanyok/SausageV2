#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_draw_parameters : require

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

{{ mesh_buffers_capacity }}
{{ mesh_uniforms }}

uniform mat4 projection_view;

void main(void) {
  mat4 transform = transforms[transform_offset[gl_BaseInstanceARB] + gl_InstanceID];
  vec4 res_position = vec4(position, 1.0);
  vec4 res_normal = vec4(normal, 0.0);
  AnimTransform(res_position, res_normal);
  Out.frag_pos = vec3(transform * res_position);
  gl_Position = projection_view * vec4(Out.frag_pos, 1.0);
  Out.base_instance = gl_BaseInstanceARB;
  Out.uv = uv;
  vec3 T = normalize(vec3(transform * vec4(tangent, 0.0)));
  vec3 B = normalize(vec3(transform * vec4(bitangent, 0.0)));
  vec3 N = normalize(vec3(transform * res_normal));
  Out.TBN = mat3(T, B, N);
}

