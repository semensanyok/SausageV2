#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require

uniform vec3 view_pos;
out vec4 color;
{{ light_enums }}
{{ light_constants }}
{{ texture_layers_offsets }}
{{ uniforms_locations }}

{{ mesh_fs_in }}

{{ light_structs }}
{{ light_uniforms }}
{{ light_functions }}

{{ texture_handle_by_mesh_id_array }}


void main(void) {
  vec4 mat_diffuse_with_opacity = texture(textures[In.base_instance], vec3(In.uv, DIFFUSE_TEX)).rgba;
  vec3 mat_diffuse = mat_diffuse_with_opacity.rgb;
  vec3 mat_specular = texture(textures[In.base_instance], vec3(In.uv, SPECULAR_TEX)).rgb;
  vec3 mat_normal = texture(textures[In.base_instance], vec3(In.uv, NORMAL_TEX)).rgb * 2.0 - 1.0;
  mat_normal = normalize(In.TBN * mat_normal);
  vec3 view_dir = normalize(view_pos - In.frag_pos);
  vec3 res = mat_diffuse * AMBIENT_CONST;
  AddLightColor(In.frag_pos, mat_normal, res, view_dir, mat_diffuse, mat_specular);
  color = vec4(res, mat_diffuse_with_opacity.a);
  //color = vec4(res, 0.04 + clamp(In.base_instance - 4, 0, 1)*0.5); //test transparency back to front sorting
//   color = vec4(255.0,0.0,0.0,0.3);
}
