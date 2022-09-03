texture = {
    "texture_array":
"""
layout (std430, binding = TEXTURE_UNIFORM_LOC) buffer TextureArray
{
    sampler2DArray textures[];
};
"""
}