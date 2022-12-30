#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require

uniform vec3 view_pos;
out vec4 color;
{{ light_enums }}
{{ light_constants }}
{{ texture_layers_offsets }}
{{ uniforms_locations }}
{{ mesh_buffers_capacity }}
{{ mesh_uniforms }}

{{ mesh_fs_in }}

{{ light_uniforms }}
{{ light_functions }}

{{ texture_handle_by_texture_id_array }}

{{ mesh_fs_functions }}

void main(void) {
  // SetBlendColor();
  //color = vec4(res, 0.04 + clamp(In.base_instance - 4, 0, 1)*0.5); //test transparency back to front sorting

  color = vec4(255.0,0.0,0.0,0.3);
}
