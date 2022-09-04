texture = {
# texture id -> texture handle.
    "texture_handle_by_mesh_id_array":
"""
layout (std430, binding = TEXTURE_HANDLE_BY_MESH_ID_ARRAY_LOC) buffer TextureHandleByMeshIdArray
{
    sampler2DArray textures[];
};
"""
}