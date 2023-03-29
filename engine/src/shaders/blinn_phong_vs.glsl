#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_draw_parameters : require

{{ uniforms_locations }}

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;
layout (location = 5) in ivec4 bone_ids; 
layout (location = 6) in vec4 bone_weights; 

uniform mat4 projection_view;

{{ mesh_buffers_capacity }}
{{ blend_textures }}
{{ mesh_uniforms }}
{{ mesh_vs_out }}

void AnimTransform(inout vec4 pos, inout vec4 norm) {
  // TODO: avoid branching, or not? 
  //       separate pass for static mesh results in 2 wavefronts anyway? 
  //       so keep it as such for now, and migrate static meshes to static_vs/fs.glsl (atm used for terrain)
  if (bone_ids[0] < 0) {
    return;
  }
  vec4 final_pos = vec4(0);
  vec4 final_norm = vec4(0);
  for (int i = 0; i < 4; i++) {
    if (bone_ids[i] < 0) {
        break;
    }
    final_pos += (bones_transforms[bone_ids[i]] * pos) * bone_weights[i];
    final_norm += (bones_transforms[bone_ids[i]] * norm) * bone_weights[i];
  }
  pos = final_pos;
  norm = final_norm;
}

void main(void) {
  {{ mesh_set_transform }}
  {{ mesh_set_res_position }}
  {{ mesh_set_res_normal }}
  //AnimTransform(res_position, res_normal);
  {{ mesh_set_vs_out }}
  {{ mesh_set_gl_Position }}
}
