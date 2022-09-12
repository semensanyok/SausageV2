mesh = {
    "mesh_fs_functions": 
"""
// TODO: choose blend color function. blend normals and specular
//       for starter just diffuse color blend
void SetBlendColor() {
    // TODO: add light color for all blend textures, before blend

    int texture_id = blend_textures[In.base_instance].textures[0]
    vec4 mat_diffuse_with_opacity = texture(textures[texture_id], vec3(In.uv, DIFFUSE_TEX)).rgba;
    vec3 mat_diffuse = mat_diffuse_with_opacity.rgb;
    vec3 mat_specular = texture(textures[texture_id], vec3(In.uv, SPECULAR_TEX)).rgb;
    vec3 mat_normal = texture(textures[texture_id], vec3(In.uv, NORMAL_TEX)).rgb * 2.0 - 1.0;
    mat_normal = normalize(In.TBN * mat_normal);

    vec3 view_dir = normalize(view_pos - In.frag_pos);
  
    vec3 res = mat_diffuse * AMBIENT_CONST;
    AddLightColor(In.frag_pos, mat_normal, res, view_dir, mat_diffuse, mat_specular);
    
    color = vec4(res, mat_diffuse_with_opacity.a);
};
""",
    "mesh_uniforms": 
"""
struct TextureBlend {
  float blend_weight;
  unsigned int texture_id;
};

struct BlendTextures {
  TextureBlend textures[MAX_BLEND_TEXTUERS];
  unsigned int num_textures;
};

layout (std430, binding = UNIFORMS_LOC) buffer UniformData
{
    mat4 bones_transforms[MAX_BONES];
    mat4 transforms[MAX_BASE_AND_INSTANCED_MESHES];
    BlendTextures blend_textures[MAX_BASE_AND_INSTANCED_MESHES];
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
"mesh_set_gl_Position":
# projection_view defined directly in shader not in template
"""
gl_Position = projection_view * vec4(Out.frag_pos, 1.0);
"""
}