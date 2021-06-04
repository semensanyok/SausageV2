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
    Specular,//                        2
    Height,  //                        3
};
struct Samplers {
    GLuint basic_repeat;
};
// --------------------------------------------------------------------------------------------------------------------
// Texture structures end

struct MeshData {
    // draw id.
    unsigned int id;
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
    Shader* shader;
    vector<BufferStorage*> buffers;
};