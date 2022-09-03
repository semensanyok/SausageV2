#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require

{{ light_enums }}
{{ texture_layers_offsets }}
{{ uniforms_locations }}

{{ light_structs }}
{{ light_uniforms }}
{{ light_functions }}

{{ mesh_fs_in }}

{{ texture_array }}

void main(void) {
  // TODO: terrain textures blend machinery. 
  //       1. diffuse; 
  //       2. normal and specular (might do it differently for normals, to preserve details from both textures, and dont multiply 0 to 1)

}