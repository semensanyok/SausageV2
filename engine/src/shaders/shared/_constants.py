constants = {
    "light_constants":
"""
const float AMBIENT_CONST = 0.1f;
const float BLINN_SPEC_POW = 32.0f;
""",
    "light_enums":
"""
const int POINT_LIGHT = 0;
const int DIRECTIONAL_LIGHT = 1;
const int SPOT_LIGHT = 2;
""",
    "texture_layers_offsets":
"""
const int DIFFUSE_TEX = 0;
const int NORMAL_TEX = 1;
const int SPECULAR_TEX = 2;
""",
    "mesh_buffers_capacity": 
"""
const uint MAX_COMMAND = 1000;
const unsigned long MAX_BASE_MESHES = 4000;
const uint MAX_BASE_AND_INSTANCED_MESHES = 4000;
const uint MAX_BONES = 100000;
const uint MAX_TRANSFORM_OFFSET = MAX_BASE_AND_INSTANCED_MESHES * 10;
const uint MAX_BLEND_TEXTUERS = 16;
const uint MAX_TEXTURE = 1000;
""",
"ui_buffers_capacity":
"""
const unsigned long MAX_3D_OVERLAY_TRANSFORM = 4000;
const unsigned long MAX_3D_OVERLAY_TRANSFORM_OFFSET = MAX_3D_OVERLAY_TRANSFORM * 10;
const unsigned long MAX_FONT_TEXTURES = 1 * TEXTURES_SINGLE_FONT;
const unsigned long MAX_UI_UNIFORM_TRANSFORM = 400;
const unsigned long MAX_UI_UNIFORM_OFFSET = MAX_UI_UNIFORM_TRANSFORM * 10;
""",
    "mesh_vertex_locations":
"""
const uint MESH_VERT_LOC_POSITION = 0;
const uint MESH_VERT_LOC_NORMAL = 1;
const uint MESH_VERT_LOC_UV = 2;
const uint MESH_VERT_LOC_TANGENT = 3;
const uint MESH_VERT_LOC_BITANGENT = 4;
const uint MESH_VERT_LOC_BONE_IDS = 5;
const uint MESH_VERT_LOC_BONE_WEIGHTS = 6;
""",
    "uniforms_locations":
"""
const uint UNIFORMS_LOC = 0;
const uint TEXTURE_HANDLE_BY_TEXTURE_ID_ARRAY_LOC = 1;
const uint LIGHTS_UNIFORM_LOC = 2;
const uint FONT_UNIFORMS_LOC = 3;
const uint FONT_TEXTURE_UNIFORM_LOC = 4;
const uint UI_UNIFORM_LOC = 5;
const uint CONTROLLER_UNIFORM_LOC = 6;
"""
}