ui = { 
    "ui_uniforms_shared" :
"""
layout (std430, binding = UI_UNIFORM_LOC) buffer UniformDataUI
{
    ivec4 min_max_x_y[MAX_UI_INSTANCES];
    vec2 transforms[MAX_UI_INSTANCES];
    float pad1[2];
    uint texture_id_by_instance_id[MAX_UI_INSTANCES];
    float pad2[3];
    uint base_instance_offset[MAX_UI_COMMANDS];
};
""",
    "ui_utils":
"""
bool IsMouseHover(int mouse_x, int mouse_y, int base_instance)
{
    ivec4 _min_max_x_y = min_max_x_y[base_instance];
    return mouse_x > _min_max_x_y.x
        && mouse_x < _min_max_x_y.y
        && mouse_y > _min_max_x_y.z
        && mouse_y < _min_max_x_y.w;
}
""",
    "3d_overlay_uniforms":
"""
layout (std430, binding = FONT_UNIFORMS_LOC) buffer FontUniformData
{
    mat4 transforms[MAX_3D_OVERLAY_INSTANCES];
    uint base_instance_offset[MAX_3D_OVERLAY_COMMANDS];
};
"""
}

