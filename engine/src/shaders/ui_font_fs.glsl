#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require

{{ uniforms_locations }}
{{ mesh_buffers_capacity }}
{{ ui_uniforms_shared }}
{{ ui_utils }}
{{ texture_handle_by_texture_id_array }}

in vs_out {
    flat int base_instance;
    flat int instance_id;
    flat int glyph_id;
    vec3 color;
    vec2 uv;
} In;

out vec4 color;

void main(void) {
  uint texture_id = texture_id_by_instance_id[base_instance_offset[In.base_instance] + In.instance_id];
  color = vec4(In.color, texture(textures[texture_id], vec3(In.uv, In.glyph_id)).r);
}
