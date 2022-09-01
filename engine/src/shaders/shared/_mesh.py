mesh = {
    "mesh_buffers_capacity": 
"""
const uint MAX_COMMAND = 1000;
const uint MAX_TRANSFORM = 4000;
const uint MAX_BONES = 100000;
const uint MAX_TRANSFORM_OFFSET = MAX_TRANSFORM * 10;
""",
    "mesh_uniforms": 
"""
layout (std430, binding = 0) buffer UniformData
{
    mat4 bones_transforms[MAX_BONES];
    mat4 transforms[MAX_TRANSFORM];
    unsigned int transform_offset[MAX_TRANSFORM_OFFSET];
};
""",

}