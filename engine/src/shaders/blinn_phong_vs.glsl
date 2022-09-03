#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_draw_parameters : require

{{ mesh_vertex_locations }}
{{ uniforms_locations }}

layout (location = MESH_VERT_LOC_POSITION) in vec3 position;
layout (location = MESH_VERT_LOC_NORMAL) in vec3 normal;
layout (location = MESH_VERT_LOC_UV) in vec2 uv;
layout (location = MESH_VERT_LOC_TANGENT) in vec3 tangent;
layout (location = MESH_VERT_LOC_BITANGENT) in vec3 bitangent;
layout (location = MESH_VERT_LOC_BONE_IDS) in ivec4 bone_ids; 
layout (location = MESH_VERT_LOC_BONE_WEIGHTS) in vec4 bone_weights; 

uniform mat4 projection_view;

{{ mesh_buffers_capacity }}
{{ mesh_uniforms }}
{{ mesh_vs_out }}



void AnimTransform(inout vec4 pos, inout vec4 norm) {
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
  AnimTransform(res_position, res_normal);
  {{ mesh_set_vs_out }}
  {{ mesh_set_gl_Position }}
}
