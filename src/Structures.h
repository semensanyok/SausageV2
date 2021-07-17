#pragma once

#include "sausage.h"
#include "Settings.h"

using namespace std;
using namespace glm;
using namespace BufferSettings;

class BufferStorage;
class Shader;
class Texture;

struct Vertex {
    vec3 Position;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Tangent;
    vec3 Bitangent;
    ivec4 BoneIds;
    vec4 BoneWeights;
};

// --------------------------------------------------------------------------------------------------------------------
// Indirect structures start
struct DrawArraysIndirectCommand
{
    GLuint count;
    GLuint primCount;
    GLuint first;
    GLuint baseInstance;
};
struct DrawElementsIndirectCommand
{
    GLuint count;
    GLuint instanceCount;
    GLuint firstIndex;
    GLuint baseVertex;
    GLuint baseInstance;
    // if randomly initialized and used - hardware crash
    DrawElementsIndirectCommand() : count{ 0 }, instanceCount{ 0 }, firstIndex{ 0 }, baseVertex{ 0 }, baseInstance{ 0 } {};
};
// Indirect structures end
// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------
// Texture structures start

enum TextureType
{
    Diffuse, // sampler2DArray   index 0
    Normal,  //                        1
    Specular,//                        2 roughness in PBR
    AO,      //                        3
    Height,  //                        4 PBR
    Metal,   //                        5 PBR
    //Opacity, //                        6 use Diffuse alpha
};

struct MaterialTexNames
{
    string diffuse;
    string normal;
    string specular;
    string height;
    string metal;
    string ao;
    string opacity;
    size_t Hash() {
        return hash<string>{}(string(diffuse).append(normal).append(specular).append(height).append(metal).append(ao).append(opacity));
    }
};
struct Samplers {
    GLuint basic_repeat;
};
// --------------------------------------------------------------------------------------------------------------------
// Texture structures end
enum LightType {
    Point,
    Directional,
    Spot
};

namespace AttenuationConsts {
    float OGRE_P_L_ATT_DIST_7L = 0.7f;
    float OGRE_P_L_ATT_DIST_7Q = 1.8f;
    float OGRE_P_L_ATT_DIST_13L = 0.35f;
    float OGRE_P_L_ATT_DIST_13Q = 0.44f;
    float OGRE_P_L_ATT_DIST_20L = 0.22f;
    float OGRE_P_L_ATT_DIST_20Q = 0.20f;
}

struct Light {
    vec4 direction;
    vec4 position;
    vec4 color;
    vec4 specular;
    
    int type;
    float spot_inner_cone_cos;
    float spot_outer_cone_cos;
    float constant_attenuation;

    float linear_attenuation;
    float quadratic_attenuation;
    float padding[2] = {0,0}; // OpenGL padding
};


struct Lights {
    int num_lights;
    float padding[3] = { 0,0,0 };
    Light lights[];
};

enum ShaderType {
    BlinnPhong,
    BRDF
};

struct Bone {
    unsigned int id;
    string name;
    /** Matrix that transforms from bone space to mesh space in bind pose.
      *
      * This matrix describes the position of the mesh
      * in the local space of this bone when the skeleton was bound.
      * Thus it can be used directly to determine a desired vertex position,
      * given the world-space transform of the bone when animated,
      * and the position of the vertex in mesh space.
      *
      * It is sometimes called an inverse-bind matrix,
      * or inverse bind pose matrix.
      * 
      * AKA bind_shape_matrix collada?
      * The bind shape matrix describes how to transform the pCylinderShape1 geometry into the right
      * coordinate system for use with the joints.  In this case we do an +90 Y transform because
      * the pCylinderShape1 geometry was initially a 180 unit long cylinder with 0,0,0 at it's center.
      * This moves it so 0,0,0 is at the base of the cylinder.
      */
    mat4 offset;
    mat4 inverse_transform;
    vector<Bone*> children;
};

struct BoneKeyFrames {
    vector<pair<double, vec3>> time_position;
    // quaternions
    vector<pair<double, quat>> time_rotation;
    vector<pair<double, vec3>> time_scale;
};

struct Animation {
    unsigned int id;
    string name;
    double duration;
    double ticks_per_second;
    double duration_seconds;

    map<string, BoneKeyFrames> bone_frames;
};

struct Armature {
    string name;
    unsigned int num_bones;
    Bone* bones;
    //mat4 transform;
    map<string, Bone*> name_to_bone;
    map<unsigned int, Bone*> id_to_bone;
    map<string, Animation*> name_to_anim;
};

struct ActiveAnimation {
    uint32_t start_time;
    float blend_factor;
    Animation* anim;
};

struct MeshData {
    unsigned long id;
    bool is_transparent;
    long buffer_id;
    unsigned long instance_id;
    unsigned long transform_offset;
    mat4 transform;
    string name;
    DrawElementsIndirectCommand command;
    BufferStorage* buffer;
    vec3 max_AABB;
    vec3 min_AABB;
    long vertex_offset;
    long index_offset;
    Texture* texture;

    MeshData* base_mesh;
    Armature* armature;

    vector<ActiveAnimation> active_animations;

    MeshData() : 
        vertex_offset{ -1 },
        index_offset{ -1 },
        buffer_id{ -1 },
        base_mesh{ nullptr },
        texture{ nullptr },
        armature{ nullptr },
        is_transparent{ false } {};
};

struct MeshLoadData {
    MeshData* mesh_data;
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    MaterialTexNames tex_names;
    unsigned int instance_count;
    //~MeshLoadData() { cout << "MeshLoadData deleted: " << (mesh_data == nullptr ? "no mesh_data" : mesh_data->name) << endl; }
};

struct BufferLock {
    mutex data_mutex;
    condition_variable is_mapped_cv;
    bool is_mapped;
    void Wait(unique_lock<mutex>& data_lock) {
        is_mapped_cv.wait(data_lock);
    }
};

struct CommandBuffer {
    GLuint id;
    DrawElementsIndirectCommand* ptr;
    unsigned int size;
    BufferLock* buffer_lock;
    bool operator==(const CommandBuffer& other)
    {
        return id == other.id;
    }
};

struct DrawCall {
    int mode = GL_TRIANGLES; // GL_TRIANGLES GL_LINES
    BufferStorage* buffer = nullptr;
    Shader* shader = nullptr;
    CommandBuffer* command_buffer;
    unsigned int command_count = 0;
    // custom data
    int num_lights = 0;
};

struct Shaders {
    Shader* blinn_phong;
    Shader* bullet_debug;
    Shader* stencil;
};

struct UniformData {
    mat4 bones_transforms[MAX_BONES];
    mat4 transforms[MAX_TRANSFORM];
    unsigned int transform_offset[MAX_TRANSFORM_OFFSET];
};