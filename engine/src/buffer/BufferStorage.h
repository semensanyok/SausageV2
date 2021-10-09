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
  const unsigned long UNIFORMS_STORAGE_SIZE = sizeof(MeshUniformData);
  const unsigned long TRANSFORM_OFFSET_STORAGE_SIZE =
      MAX_TRANSFORM_OFFSET * sizeof(unsigned int);
  const unsigned long TEXTURE_STORAGE_SIZE = MAX_TEXTURES * sizeof(GLuint64);
  
  const GLbitfield flags =
      GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
  const int UNIFORMS_LOC = 0;
  const int TEXTURE_UNIFORM_LOC = 1;
  const int LIGHTS_UNIFORM_LOC = 2;
  const int FONT_UNIFORMS_LOC = 3;
  const int FONT_TEXTURE_UNIFORM_LOC = 4;
  const int FONT_UNIFORMS_UI_LOC = 5;

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
  MeshUniformData *uniforms_ptr;
  GLuint64 *texture_ptr;
  Lights *light_ptr;

  float allocated_percent_vertex = 0.0;
  float allocated_percent_index = 0.0;

  BufferType::BufferTypeFlag bound_buffers;
  
  // FONT buffers
  const unsigned long FONT_TEXTURE_STORAGE_SIZE = MAX_FONT_TEXTURES * sizeof(GLuint64);
  const unsigned long FONT_UNIFORMS_STORAGE_SIZE = sizeof(FontUniformData);
  const unsigned long FONT_TRANSFORM_OFFSET_STORAGE_SIZE = MAX_FONT_TRANSFORM_OFFSET * sizeof(unsigned int);
  const unsigned long FONT_UNIFORMS_UI_STORAGE_SIZE = sizeof(FontUniformDataUI);
  const unsigned long FONT_TRANSFORM_OFFSET_UI_STORAGE_SIZE = MAX_FONT_UI_TRANSFORM_OFFSET * sizeof(unsigned int);
  GLuint font_texture_buffer;
  GLuint64 *font_texture_ptr;
  GLuint font_uniforms_buffer;
  GLuint font_uniforms_ui_buffer;
  FontUniformData* font_uniforms_ptr;
  FontUniformDataUI* font_uniforms_ui_ptr;
public:
  unsigned long transforms_total;
  unsigned long transforms_total_font;
  unsigned long transforms_total_font_ui;

  int id = -1;

  GLsync fence_sync = 0;

  bool bind_draw_buffer = true;
  BufferStorage() {
    static int count = 0;
    id = count++;
    transforms_total = 0;
    bound_buffers = 0;
  };
  ~BufferStorage(){};
  void Reset() {
    fence_sync = 0;
    vertex_start = 0;
    index_total = 0;
    transforms_total = 0;
    bound_buffers = 0;
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
  int AddCommand(DrawElementsIndirectCommand &command, CommandBuffer *buf,
                int command_offset = -1);
  void BufferMeshData(vector<MeshDataBase*>& load_data_meshes,
                      vector<shared_ptr<MeshLoadData>>& load_data,
                      unsigned long& vertex_total,
                      unsigned long& index_total,
                      unsigned long& meshes_total,
                      BufferMargins& margins);
  void BufferMeshData(MeshDataBase* mesh,
                      shared_ptr<MeshLoadData> load_data,
                      unsigned long& vertex_total,
                      unsigned long& index_total,
                      unsigned long& meshes_total,
                      BufferMargins& margins,
                      vector<MeshDataBase *> &instances = SausageDefaults::DEFAULT_MESH_DATA_VECTOR);
  void BufferBoneTransform(map<unsigned int, mat4> &id_to_transform);
  void BufferBoneTransform(Bone *bone, mat4 &trans, unsigned int num_bones = 1);
  void BufferTransform(MeshData *mesh);
  void BufferTransform(vector<MeshData *> &mesh);
  void BufferLights(vector<Light *> &lights);
  void InitMeshBuffers();
  void BindVAOandBuffers(BufferType::BufferTypeFlag buffers_to_bind);
  void Dispose();

  void BufferMeshTexture(MeshData* mesh) {
    if (mesh->texture != nullptr) {
      texture_ptr[mesh->buffer_id] = mesh->texture->texture_handle_ARB;
    }
  }
  void BufferFontTexture(MeshDataBase *mesh, Texture *texture) {
    font_texture_ptr[mesh->buffer_id] = texture->texture_handle_ARB;
  };
  void Buffer3DFontTransform(MeshDataFont3D *mesh) {
    if (mesh->transform_offset == -1) {
      mesh->transform_offset = _GetTransformBucketFont(mesh);
    }
    font_uniforms_ptr->transforms[mesh->transform_offset + mesh->instance_id] =
        mesh->transform;
    if (mesh->instance_id == 0) {
      font_uniforms_ptr->transform_offset[mesh->buffer_id + mesh->instance_id] =
          mesh->transform_offset;
    }
    is_need_barrier = true;
  };
  void BufferUIFontTransform(MeshDataFontUI* mesh) {
    if (mesh->transform_offset == -1) {
      mesh->transform_offset = _GetTransformBucketFontUI(mesh);
    }
    font_uniforms_ui_ptr->transforms[mesh->transform_offset + mesh->instance_id] =
        mesh->transform;
    if (mesh->instance_id == 0) {
      font_uniforms_ui_ptr->transform_offset[mesh->buffer_id + mesh->instance_id] =
          mesh->transform_offset;
    }
    is_need_barrier = true;
  };

 private:
  long _GetTransformBucket(MeshData *mesh);
  long _GetTransformBucketFont(MeshDataFont3D *mesh);
  long _GetTransformBucketFontUI(MeshDataFontUI *mesh);
};
