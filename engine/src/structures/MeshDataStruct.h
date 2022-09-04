#pragma once

#include "Structures.h"
#include "GPUStructs.h"
#include "Interfaces.h"
#include "TextureStruct.h"

using namespace std;
using namespace glm;

class   Texture;
class   Armature;
class   PhysicsData;

struct Vertex {
  vec3 Position;
  vec3 Normal;
  vec2 TexCoords;
  vec3 Tangent;
  vec3 Bitangent;
  ivec4 BoneIds;
  vec4 BoneWeights;
};

struct MeshLoadData {
  vector<Vertex> vertices;
  vector<unsigned int> indices;
  MaterialTexNames* tex_names;
  Armature* armature;
  PhysicsData* physics_data;
  string name;
  mat4 transform;
  unsigned int instance_count;
};

class MeshDataBase {
public:
  unsigned long id;
  long buffer_id;
  unsigned long instance_id;
  long vertex_offset;
  long index_offset;
  long transform_offset;
  MeshDataBase* base_mesh;
  DrawElementsIndirectCommand command;
  MeshDataBase()
    : vertex_offset{ -1 },
    index_offset{ -1 },
    buffer_id{ -1 },
    base_mesh{ nullptr },
    transform_offset{ -1 } {};
  virtual ~MeshDataBase() {};
};

class MeshData : public MeshDataBase, public SausageUserPointer {
  friend class MeshManager;
  friend class MeshDataClickable;
public:
  mat4 transform;
  bool is_transparent;

  string name;
  Texture* texture;
  Armature* armature;
  PhysicsData* physics_data;

private:
  MeshData()
    : texture{ nullptr },
    physics_data{ nullptr },
    is_transparent{ false } {};
  MeshData(MeshLoadData* load_data)
    : texture{ nullptr },
    physics_data{ load_data->physics_data },
    armature{ load_data->armature },
    name{ load_data->name },
    transform{ load_data->transform },
    is_transparent{ false } {};
  ~MeshData() {
    delete physics_data;
  };
};

class MeshDataClickable : public SausageUserPointer {
public:
  MeshData* mesh_data;
  MeshDataClickable(MeshData* mesh_data) : mesh_data{ mesh_data } {};
  void Call() {
    cout << "RayHit from mesh " << mesh_data->name << endl;
  }
};

class MeshDataUI : public MeshDataBase {
  friend class MeshManager;
public:
  vec2 transform;
  Texture* texture;
private:
  MeshDataUI() : texture{ nullptr } {};
  MeshDataUI(vec2 transform, Texture* texture) :
    transform{ transform },
    texture{ texture } {};
  ~MeshDataUI() {};
};

class MeshDataOverlay3D : public MeshDataBase {
  friend class MeshManager;
public:
  string text;
  mat4 transform;
  Texture* texture;
private:
  MeshDataOverlay3D(string& text, mat4& transform) : text{ text }, transform{ transform }, texture{ nullptr } {};
  MeshDataOverlay3D() : texture{ nullptr } {};
  ~MeshDataOverlay3D() {};
};
