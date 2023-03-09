texture = {
# texture id -> texture handle.
    "texture_handle_by_texture_id_array":
"""
layout (std430, binding = TEXTURE_LOC) buffer TextureHandleByMeshIdArray
{
    sampler2DArray textures[];
};
""",
    "blend_textures":
"""
struct TextureBlend {
  float blend_weight;
  uint texture_id;
};

struct BlendTextures {
  TextureBlend textures[MAX_BLEND_TEXTURES];
  uint num_textures;
};
"""
}