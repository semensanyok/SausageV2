#pragma once

#include "sausage.h"


using namespace std;
using namespace glm;

class BufferStorage;
class Shader;
class Texture;

struct Vertex {
    // position
    vec3 Position;
    // normal
    vec3 Normal;
    // texCoords
    vec2 TexCoords;
    // tangent
    vec3 Tangent;
    // bitangent
    vec3 Bitangent;
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
    Opacity, //                        6 PBR
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
struct MeshData {
    // draw id.
    unsigned long id;
    unsigned long buffer_id;
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
    MeshData() : vertex_offset{ -1 }, index_offset{ -1 } {};
};

struct MeshLoadData {
    MeshData* mesh_data;
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    MaterialTexNames tex_names;
    unsigned int instance_count;
    //~MeshLoadData() { cout << "MeshLoadData deleted: " << (mesh_data == nullptr ? "no mesh_data" : mesh_data->name) << endl; }
};

struct DrawCall {
    BufferStorage* buffer;
    Shader* shader;
    unsigned int command_count;
    unsigned int command_offset; // offset in BufferStorage command buffer
    
    // custom data
    int num_lights;
};