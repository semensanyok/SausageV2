#pragma once

#include "Logging.h"
#include "OpenGLHelpers.h"
#include "Settings.h"
#include "MeshDataStruct.h"
#include "AnimationStruct.h"
#include "GPUUniformsStruct.h"
#include "TextureStruct.h"
#include "LightStruct.h"
#include "Texture.h"
#include "sausage.h"
#include "Macros.h"


using namespace std;
using namespace glm;
using namespace BufferSettings;
using namespace UniformsLocations;
using namespace BufferSizes;

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

// TODO: try coroutines instead of locks

class BufferStorage {
  friend class MeshDataBufferConsumer;
  friend class FontBufferConsumerUI;
  friend class OverlayBufferConsumer3D;

 private:
  const GLbitfield flags =
      GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

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
  GLuint blend_textures_by_mesh_id_buffer;
  GLuint texture_handle_by_texture_id_buffer;
  GLuint light_buffer;
  vector<CommandBuffer *> command_buffers;
  unordered_map<GLuint, CommandBuffer *> mapped_command_buffers;
  //////////////////////////
  // Mapped buffers pointers
  //////////////////////////
  Vertex *vertex_ptr;
  unsigned int *index_ptr;
  MeshUniform *uniforms_ptr;
  BlendTexturesByMeshIdUniform* blend_textures_by_mesh_id_ptr;
  // gl_BaseInstanceARB->texture handle.textures of same size, 4 layers
  GLuint64 *texture_handle_by_texture_id_ptr;
  LightsUniform *light_ptr;

  float allocated_percent_vertex = 0.0;
  float allocated_percent_index = 0.0;

  BufferType::BufferTypeFlag bound_buffers;

  GLuint font_texture_buffer;
  GLuint64 *font_texture_ptr;
  GLuint uniforms_3d_overlay_buffer;
  GLuint uniforms_ui_buffer;
  GLuint uniforms_controller_buffer;
  UniformData3DOverlay *uniforms_3d_overlay_ptr;
  UniformDataUI *uniforms_ui_ptr;
  ControllerUniformData* uniforms_controller_ptr;

 public:
  unsigned long transforms_total = 0;
  unsigned long transforms_total_font = 0;
  unsigned long transforms_total_font_ui = 0;

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
    transforms_total_font = 0;
    transforms_total_font_ui = 0;
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
  void BufferMeshData(vector<MeshDataBase *> &load_data_meshes,
                      vector<shared_ptr<MeshLoadData>> &load_data,
                      unsigned long &vertex_total, unsigned long &index_total,
                      unsigned long &meshes_total, BufferMargins &margins);
  void BufferMeshData(MeshDataBase* mesh, shared_ptr<MeshLoadData> load_data,
                      unsigned long& vertex_total, unsigned long& index_total,
                      unsigned long& meshes_total, BufferMargins& margins);
  void BufferMeshData(MeshDataBase* mesh, shared_ptr<MeshLoadData> load_data,
                      unsigned long &vertex_total, unsigned long &index_total,
                      unsigned long &meshes_total, BufferMargins &margins,
                      vector<MeshDataBase *> &instances);
  void BufferBoneTransform(unordered_map<unsigned int, mat4> &id_to_transform);
  void BufferBoneTransform(Bone *bone, mat4 &trans, unsigned int num_bones = 1);
  void BufferTransform(MeshData *mesh);
  void BufferTransform(vector<MeshData *> &mesh);
  void BufferLights(vector<Light *> &lights);
  void InitMeshBuffers();
  void BindVAOandBuffers(BufferType::BufferTypeFlag buffers_to_bind);
  void Dispose();

  void BufferTextureHandle(Texture* texture);
  void BufferMeshTexture(MeshData* mesh);
  void BufferFontTexture(MeshDataBase* mesh, Texture* texture);
  void Buffer3DFontTransform(MeshDataOverlay3D* mesh);;
  void BufferUniformDataUISize(MeshDataUI* mesh, int min_x, int max_x, int min_y, int max_y);
  void BufferUniformDataUITransform(MeshDataUI* mesh);;
  void BufferUniformDataController(int mouse_x, int mouse_y, int is_pressed, int is_click);

 private:
  long _GetTransformBucket(MeshData *mesh);
  long _GetTransformBucketFont(MeshDataOverlay3D *mesh);
  long _GetTransformBucketFontUI(MeshDataUI *mesh);
};
