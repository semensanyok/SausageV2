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


struct Mesh {
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    unsigned int id;
    unsigned int VAO, VBO, EBO;
};

struct MeshUniforms
{
    mat4     view;
    mat4     projection;
    mat4     model[];
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

struct MeshDrawIds {
    unsigned int texture_diffuse_id;
    unsigned int texture_normal_id;
    unsigned int texture_specular_id;
    unsigned int texture_height_id;
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
    unsigned int basic_repeat;
};

struct Texture {
    unsigned int id;
    const char* name;
    TextureType type;
};
// --------------------------------------------------------------------------------------------------------------------
// Texture structures end