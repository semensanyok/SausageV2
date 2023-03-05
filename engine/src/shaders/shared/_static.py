
static_mesh = {
"static_mesh_buffers_capacity":{
"""
const unsigned long MAX_VERTEX_STATIC = 1000000;
const unsigned long MAX_INDEX_STATIC = 2 * MAX_VERTEX_STATIC;
const unsigned long MAX_BASE_MESHES_STATIC = 40;
const unsigned long MAX_MESHES_STATIC_INSTANCES = MAX_BASE_MESHES * 10;
"""
},
"static_mesh_uniforms":{
"""
struct UniformDataStaticMesh {
  mat4 transforms[MAX_MESHES_STATIC_INSTANCES];
  BlendTextures blend_textures[MAX_MESHES_STATIC_INSTANCES];
  float pad[3];
  unsigned int base_instance_offset[MAX_BASE_MESHES_STATIC];
};
"""
},
#################!!! CURRENTLY REUSED _MESH FUNCTIONS IN SHADER ##############################
#     "static_mesh_vs_out": 
# """
# out vs_out {
#     flat uint base_instance;
#     flat uint instance_id;
#     vec2 uv;
#     vec3 frag_pos;
#     mat3 TBN;
# } Out;    
# """,
#     "static_mesh_fs_in": 
# """
# in vs_out {
#     flat uint base_instance;
#     flat uint instance_id;
#     vec2 uv;
#     vec3 frag_pos;
#     mat3 TBN;
# } In;
# """,
# "static_mesh_set_transform":
# """
# mat4 transform = transforms[base_instance_offset[gl_BaseInstanceARB] + gl_InstanceID];
# // mat4 transform = mat4(1.0);
# """,
#############################################################################################

}