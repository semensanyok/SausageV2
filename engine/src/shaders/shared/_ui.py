ui_transform = { "ui_transform_shared" :
"""
const uint MAX_TRANSFORM = 400;
layout (std430, binding = 5) buffer UniformDataUI
{
    ivec4 min_max_x_y[MAX_TRANSFORM];
    vec2 transforms[MAX_TRANSFORM];
};
"""
}