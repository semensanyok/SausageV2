#pragma once

#include "sausage.h"
using namespace std;
using namespace glm;

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
enum class TextureType
{
    Diffuse,
    Normal,
    Specular,
    Height,
};

struct Samplers {
    GLuint basic_repeat;
};

struct Texture {
    unsigned int id;
    const char* name;
    TextureType type;
};
// --------------------------------------------------------------------------------------------------------------------
// Texture structures end

// OLD. switched to bindless multidraw /////////////////////////////////////////
// struct Mesh {
//     vector<Vertex> vertices;
//     vector<unsigned int> indices;
//     unsigned int draw_id;
//     unsigned int VAO, VBO, EBO;
// };

struct MeshData {
    unsigned int draw_id;
};

struct MeshLoadData {
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    unsigned int draw_id;
};

// indices in shader for texture sampler array
struct MeshShaderIds {
    unsigned int texture_diffuse_id;
    unsigned int texture_normal_id;
    unsigned int texture_specular_id;
    unsigned int texture_height_id;
};