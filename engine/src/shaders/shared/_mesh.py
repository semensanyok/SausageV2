mesh = {
    "mesh_uniforms": 
"""
layout (std430, binding = UNIFORMS_LOC) buffer UniformData
{
    mat4 bones_transforms[MAX_BONES];
    mat4 transforms[MAX_TRANSFORM];
    unsigned int transform_offset[MAX_TRANSFORM_OFFSET];
};
""",
    "mesh_vs_out": 
"""
out vs_out {
    int base_instance;
    vec2 uv;
    vec3 frag_pos;
    mat3 TBN;
} Out;    
""",
    "mesh_fs_in": 
"""
in vs_out {
    flat int base_instance;
    vec2 uv;
    vec3 frag_pos;
    mat3 TBN;
} In;
""",
# MAIN FUNCIONS, mesh_set_VARIABLE_NAME
"mesh_set_transform":
"""
mat4 transform = transforms[transform_offset[gl_BaseInstanceARB] + gl_InstanceID];
""",
"mesh_set_res_position":
"""
vec4 res_position = vec4(position, 1.0);
""",
"mesh_set_res_normal":
"""
vec4 res_normal = vec4(normal, 0.0);
""",
"mesh_set_vs_out":
"""
Out.frag_pos = vec3(transform * res_position);
Out.base_instance = gl_BaseInstanceARB;
Out.uv = uv;
vec3 T = normalize(vec3(transform * vec4(tangent, 0.0)));
vec3 B = normalize(vec3(transform * vec4(bitangent, 0.0)));
vec3 N = normalize(vec3(transform * res_normal));
Out.TBN = mat3(T, B, N);
""",
# projection_view defined directly in shaderm not in template
"mesh_set_gl_Position":
"""
gl_Position = projection_view * vec4(Out.frag_pos, 1.0);
"""
}