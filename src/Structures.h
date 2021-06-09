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
    string diffuse_name;
    string normal_name;
    string specular_name;
    string height_name;
    string metal_name;
    string ao_name;
    string opacity_name;
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
struct Light {
    vec3 direction;
    vec3 position;
    float spot_max_angle;
    vec3 color;
    unsigned int light_type;
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