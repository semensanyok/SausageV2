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
  const uint MAX_BASE_MESHES = 400;
  const uint MAX_MESHES_INSTANCES = MAX_BASE_MESHES * 10;
  const uint MAX_3D_OVERLAY_COMMANDS = 100;
  const uint MAX_3D_OVERLAY_INSTANCES = MAX_3D_OVERLAY_COMMANDS;
  const uint MAX_UI_COMMANDS = 5;
  const uint MAX_UI_INSTANCES = 5;
  const uint MAX_BONES = 100;
  const uint MAX_TEXTURE = 100;
  const uint MAX_BLEND_TEXTURES = 4;
  const uint MAX_LIGHTS = 100;
""",
    "uniforms_locations":
"""
const uint UNIFORMS_LOC = 0;
const uint TEXTURE_HANDLE_BY_TEXTURE_ID_ARRAY_LOC = 1;
const uint LIGHTS_UNIFORM_LOC = 2;
const uint FONT_UNIFORMS_LOC = 3;
const uint UI_UNIFORM_LOC = 4;
const uint CONTROLLER_UNIFORM_LOC = 5;
const uint BLEND_TEXTURES_BY_MESH_ID_LOC = 6;
"""
}