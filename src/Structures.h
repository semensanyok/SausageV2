#pragma once

#include "sausage.h"

using namespace std;
using namespace glm;

class BufferStorage;
class Shader;

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

float OGRE_P_L_ATT_DIST_7L = 0.7f;
float OGRE_P_L_ATT_DIST_7Q = 1.8f;
float OGRE_P_L_ATT_DIST_13L = 0.35f;
float OGRE_P_L_ATT_DIST_13Q = 0.44f;
float OGRE_P_L_ATT_DIST_20L = 0.22f;
float OGRE_P_L_ATT_DIST_20Q = 0.20f;

struct Light {
    vec3 direction;
    vec3 position;
    float spot_max_angle;
    vec3 color;
    vec3 specular;
    unsigned int type;
    float constant_attenuation;
    float linear_attenuation;
    float quadratic_attenuation;
};
struct Lights {
    int num_lights;
    Light lights[];
};
enum ShaderType {
    BlinnPhong,
    BRDF
};
struct MeshData {
    // draw id.
    unsigned int id;
    //MeshType type;
    mat4 model;
    DrawElementsIndirectCommand command;
    BufferStorage* buffer;
    // aabb params
    // 
};

struct MeshLoadData {
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    unsigned int draw_id;
};

struct DrawCall {
    BufferStorage* buffer;
    Shader* shader;
    unsigned int command_count;
    unsigned int command_offset; // offset in BufferStorage command buffer
};