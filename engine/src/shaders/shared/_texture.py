texture = {
# texture id -> texture handle.
    "texture_handle_by_texture_id_array":
"""
layout (std430, binding = TEXTURE_HANDLE_BY_TEXTURE_ID_ARRAY_LOC) buffer TextureHandleByMeshIdArray
{
    sampler2DArray textures[];
};
""",
    "blend_textures_by_mesh_id":
"""
struct TextureBlend {
  float blend_weight;
  uint texture_id;
};

struct BlendTextures {
  TextureBlend textures[MAX_BLEND_TEXTURES];
  uint num_textures;
};

layout (std430, binding = BLEND_TEXTURES_BY_MESH_ID_LOC) buffer BlendTexturesByMeshIdUniform
{
    BlendTextures blend_textures[MAX_BASE_AND_INSTANCED_MESHES];
};
"""
}