ui = { "ui_uniforms_shared" :
"""
const uint MAX_TRANSFORM = 400;
layout (std430, binding = 5) buffer UniformDataUI
{
    ivec4 min_max_x_y[MAX_TRANSFORM];
    vec2 transforms[MAX_TRANSFORM];
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
"""
}

