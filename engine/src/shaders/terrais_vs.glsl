#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_draw_parameters : require

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

uniform mat4 projection_view;

{{ mesh_buffers_capacity }}
{{ mesh_uniforms }}
{{ mesh_vs_out }}


void main(void) {
  {{ mesh_set_transform }}
  {{ mesh_set_res_position }}
  {{ mesh_set_res_normal }}
  {{ mesh_set_vs_out }}
  {{ mesh_set_gl_Position }}
}

