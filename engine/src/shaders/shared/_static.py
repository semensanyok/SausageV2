static_mesh = {
  "mesh_uniforms":
"""
layout (std430, binding = MESH_STATIC_UNIFORMS_LOC) buffer UniformDataStaticMesh {
  mat4 transforms[MAX_MESHES_STATIC_INSTANCES];
  BlendTextures blend_textures[MAX_MESHES_STATIC_INSTANCES];
  float pad[3];
  unsigned int base_instance_offset[MAX_BASE_MESHES_STATIC];
};
""",
"mesh_vs_out": 
"""
out vs_out {
    flat uint base_instance;
    flat uint instance_id;
    vec2 uv;
    vec3 frag_pos;
    mat3 TBN;
    flat uint uniform_id;
} Out; 
""",
"mesh_fs_in": 
"""
in vs_out {
    flat uint base_instance;
    flat uint instance_id;
    vec2 uv;
    vec3 frag_pos;
    mat3 TBN;
    flat uint uniform_id;
} In;
""",
"mesh_set_vs_out":
"""
Out.frag_pos = vec3(transform * res_position);
Out.base_instance = gl_BaseInstanceARB;
Out.instance_id = gl_InstanceID;
Out.uv = uv;
vec3 T = normalize(vec3(transform * vec4(tangent, 0.0)));
vec3 B = normalize(vec3(transform * vec4(bitangent, 0.0)));
vec3 N = normalize(vec3(transform * res_normal));
Out.TBN = mat3(T, B, N);

Out.uniform_id = uniform_id;
"""
}
