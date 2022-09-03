#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_draw_parameters : require

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;
layout (location = 5) in ivec4 bone_ids; 
layout (location = 6) in vec4 bone_weights; 

{{ mesh_buffers_capacity }}
{{ mesh_uniforms }}

uniform mat4 projection_view;

out vs_out {
    int base_instance;
    vec2 uv;
    vec3 frag_pos;
    mat3 TBN;
} Out;

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
