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
{{ blend_textures }}
{{ mesh_uniforms }}

{{ mesh_fs_in }}

{{ light_uniforms }}
{{ light_functions }}

{{ texture_handle_by_texture_id_array }}

{{ mesh_fs_functions }}

// vec3 GetInstCol() {
    
//   vec3 inst_col;
//   if (In.base_instance == 0) {
//     inst_col = vec3(255, 0, 0);
//   }
//   if (In.base_instance == 1) {
//     inst_col = vec3(0, 255, 0);
//   }
//   if (In.base_instance == 2) {
//     inst_col = vec3(0, 0, 255);
//   }
//   if (In.base_instance == 3) {
//     inst_col = vec3(255, 255, 0);
//   }
//   if (In.base_instance == 4) {
//     inst_col = vec3(255, 0, 255);
//   }
//   if (In.base_instance == 5) {
//     inst_col = vec3(0, 255, 255);
//   }
//   if (In.base_instance == 6) {
//     inst_col = vec3(255, 255, 255);
//   }
//   return inst_col;
// }

void main(void) {
  SetBlendColor();
  //color = vec4(res, 0.04 + clamp(In.base_instance - 4, 0, 1)*0.5); //test transparency back to front sorting

  // color = vec4(255.0,0.0,0.0,0.3);
  
  // color = vec4(int(In.instance_id == 0) * 255.0, int(In.instance_id == 1) * 255.0, int(In.instance_id == 2) * 255.0,0.3);
  // color = vec4(int(In.base_instance == 1) * 255.0, int(In.base_instance == 3) * 255.0, int(In.instance_id == 2) * 255.0,0.3);
  
  // vec3 inst_col = GetInstCol();
  // color = vec4(inst_col[0],inst_col[1],inst_col[2],0.3);
}
