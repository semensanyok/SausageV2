#pragma once

#include "sausage.h"
#include "Logging.h"
#include "OpenGLHelpers.h"
#include "Settings.h"
#include "Structures.h"
#include "Texture.h"

using namespace std;
using namespace glm;
using namespace BufferSettings;

// struct DataRangeLock {
//    unsigned int vertex_begin;
//    unsigned int vertex_end;
//    unsigned int index_begin;
//    unsigned int index_end;
//    unsigned int transform_begin;
//    unsigned int transform_end;
//
//    mutex data_mutex;
//    condition_variable is_vertex_buffer_mapped;
//    bool is_mapped;
//};

class BufferStorage {
  friend class MeshDataBufferConsumer;
  friend class FontBufferConsumer;

private:
  const unsigned long VERTEX_STORAGE_SIZE = MAX_VERTEX * sizeof(Vertex);
  const unsigned long INDEX_STORAGE_SIZE = MAX_INDEX * sizeof(unsigned int);
  const unsigned long COMMAND_STORAGE_SIZE =
      MAX_COMMAND * sizeof(DrawElementsIndirectCommand);
  const unsigned long LIGHT_STORAGE_SIZE = MAX_LIGHTS * sizeof(Light);
  ///////////
  // UNIFORMS
  ///////////
  const unsigned long UNIFORMS_STORAGE_SIZE = sizeof(UniformData);
  const unsigned long TRANSFORM_OFFSET_STORAGE_SIZE =
      MAX_TRANSFORM_OFFSET * sizeof(unsigned int);
  const unsigned long TEXTURE_STORAGE_SIZE = MAX_TEXTURES * sizeof(GLuint64);

  const GLbitfield flags =
      GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
  const int UNIFORMS_LOC = 0;
  const int TEXTURE_UNIFORM_LOC = 1;
  const int LIGHTS_UNIFORM_LOC = 2;

  bool is_need_barrier = false;

  unsigned long vertex_start = 0;
  unsigned long index_total = 0;
  unsigned int textures_total = 0;

  ///////////
  /// Buffers
  ///////////
  GLuint mesh_VAO;
  GLuint vertex_buffer;
  GLuint index_buffer;
  /////////////////////
  // UNIFORMS AND SSBO
  /////////////////////
  GLuint uniforms_buffer;
  GLuint texture_buffer;
  GLuint light_buffer;
  vector<CommandBuffer *> command_buffers;
  map<GLuint, CommandBuffer *> mapped_command_buffers;
  //////////////////////////
  // Mapped buffers pointers
  //////////////////////////
  Vertex *vertex_ptr;
  unsigned int *index_ptr;
  UniformData *uniforms_ptr;
  GLuint64 *texture_ptr;
  Lights *light_ptr;

  float allocated_percent_vertex;
  float allocated_percent_index;

public:
  unsigned long transforms_total;

  int id = -1;

  GLsync fence_sync = 0;

  bool bind_draw_buffer = true;
  BufferStorage() {
    static int count = 0;
    id = count++;
    transforms_total = 0;
  };
  ~BufferStorage(){};
  void Reset() {
    fence_sync = 0;
    vertex_start = 0;
    index_total = 0;
    transforms_total = 0;
  };
  BufferMargins RequestStorage(float buffer_part_percent_vertex,
                               float buffer_part_percent_index);
  bool operator<(const BufferStorage &other) { return id < other.id; }
  bool operator==(const BufferStorage &other) { return id == other.id; }

  // map buffers that updated asyncronously
  void MapBuffers();
  void MapBuffer(CommandBuffer *buf);

  void WaitGPU(GLsync fence_sync,
               const source_location &location = source_location::current());
  void SyncGPUBufAndUnmap();
  void UnmapBuffer(CommandBuffer *buf);
  CommandBuffer *CreateCommandBuffer(unsigned int size);
  void ActivateCommandBuffer(CommandBuffer *buf);
  void RemoveCommandBuffer(CommandBuffer *to_remove);
  void DeleteCommandBuffer(CommandBuffer *to_delete);
  int AddCommands(vector<DrawElementsIndirectCommand> &active_commands,
                  CommandBuffer *buf, int command_offset = -1);
  void BufferMeshData(vector<shared_ptr<MeshLoadData>>& load_data,
      unsigned long& vertex_total,
      unsigned long& index_total,
      unsigned long& meshes_total,
      BufferMargins& margins,
      bool is_transform_used = true);
  void BufferMeshData(shared_ptr<MeshLoadData> load_data,
      unsigned long& vertex_total,
      unsigned long& index_total,
      unsigned long& meshes_total,
      BufferMargins& margins,
      vector<MeshData*>& instances = SausageDefaults::DEFAULT_MESH_DATA_VECTOR,
      bool is_transform_used = true);
 int AddCommand(DrawElementsIndirectCommand &command, CommandBuffer *buf,
                int command_offset = -1);
  void BufferBoneTransform(map<unsigned int, mat4> &id_to_transform);
  void BufferBoneTransform(Bone *bone, mat4 &trans, unsigned int num_bones = 1);
  void BufferTransform(MeshData *mesh);
  void BufferTransform(vector<MeshData *> &mesh);
  void BufferLights(vector<Light *> &lights);
  void InitMeshBuffers();
  void BindMeshVAOandBuffers();
  void Dispose();
};
