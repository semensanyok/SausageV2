terrain = {
  "mesh_uniforms":
"""
layout (std430, binding = MESH_TERRAIN_UNIFORMS_LOC) buffer UniformDataTerrainMesh {
  mat4 transforms[MAX_MESHES_TERRAIN];
  BlendTextures blend_textures[TERRAIN_PATCH_MAX_TEX_BLENDS];
  float pad[3];
  uint base_instance_offset[MAX_MESHES_TERRAIN];
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
// id for blend texture's array.
// terrain chunk is contigious mesh, where each tile referenced by vertex value (uniform_id)
Out.base_instance = uniform_id;
Out.instance_id = 0;
Out.uv = uv;
vec3 T = normalize(vec3(transform * vec4(tangent, 0.0)));
vec3 B = normalize(vec3(transform * vec4(bitangent, 0.0)));
vec3 N = normalize(vec3(transform * res_normal));
Out.TBN = mat3(T, B, N);

Out.uniform_id = uniform_id;
"""
}
