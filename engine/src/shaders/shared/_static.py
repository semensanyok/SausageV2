static_mesh = {
  "mesh_uniforms":
"""
layout (std430, binding = MESH_STATIC_UNIFORMS_LOC) buffer UniformDataStaticMesh {
  mat4 transforms[MAX_MESHES_STATIC_INSTANCES];
  BlendTextures blend_textures[MAX_MESHES_STATIC_INSTANCES];
  float pad[3];
  unsigned int base_instance_offset[MAX_BASE_MESHES_STATIC];
};
"""
}
