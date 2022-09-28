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
#include "ThreadSafeSet.h"
#include "ThreadSafeNumberPool.h"
#include "Arena.h"

/**
Only command buffer must be contigious
(to issue drawcall with multiple commands they must be packed together)
Vertex/Index buffers take offsets from commands, can be allocated anywhere.

TODO:
Slot release logic on deleted BaseMesh/Command/Vertex/Index etc.
maybe predefined slot types of same size, to avoid fragmentation

try coroutines instead of locks
*/
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

class BufferStorage {
  //friend class MeshDataBufferConsumer;
  //friend class FontBufferConsumerUI;
  //friend class OverlayBufferConsumer3D;
  friend class Renderer;

 private:
  const GLbitfield flags =
      GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

  bool is_need_barrier = false;

  ThreadSafeNumberPool* transforms_total;
  ThreadSafeNumberPool* transforms_total_font;
  ThreadSafeNumberPool* transforms_total_font_ui;
  //  TODO: each drawcall uses contigious range of commands. Need to allocate in advance for shader.
  //    or place shader with dynamic number of meshes at the end
  Arena* index_arena;
  Arena* vertex_arena;

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
  CommandBuffer* command_buffer;
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

  BufferType::BufferTypeFlag bound_buffers;

  GLuint font_texture_buffer;
  GLuint64 *font_texture_ptr;
  GLuint uniforms_3d_overlay_buffer;
  GLuint uniforms_ui_buffer;
  GLuint uniforms_controller_buffer;
  UniformData3DOverlay *uniforms_3d_overlay_ptr;
  UniformDataUI *uniforms_ui_ptr;
  ControllerUniformData* uniforms_controller_ptr;

  BufferType::BufferTypeFlag used_buffers;

 public:

  GLsync fence_sync = 0;

  BufferStorage() {
    transforms_total = new ThreadSafeNumberPool(MAX_BASE_AND_INSTANCED_MESHES);
    transforms_total_font = new ThreadSafeNumberPool(MAX_3D_OVERLAY_TRANSFORM);
    transforms_total_font_ui = new ThreadSafeNumberPool(MAX_UI_UNIFORM_TRANSFORM);

    // TODO: better "memory slots" for indices/vertices/commands
    //       with fixed size (SMALL(<10 <100 verts?)/MEDIUM(<10000 verts)/LARGE(==100 000+) verts slots)
    //       to avoid memory fragmentation and limitless buffer growth
    //  - each drawcall uses contigious range of commands. Need to allocate in advance for shader.
    //    or place shader with dynamic number of meshes at the end

    /**
    * had idea to have multiple arenas for each storage
    * to store large objects in one, smaller in other
    * now for simplicity - keep 1 Arena per buffer(with offset = 0)
    **/
    unsigned int offset = 0;
    Arena* index_arena = new Arena(MAX_INDEX, offset);
    Arena* vertex_arena = new Arena(MAX_VERTEX, offset);
    // includes instances (TODO: reserve some for terrain patches. when exceed - add new command or resize?)
  };
  ~BufferStorage(){
    delete transforms_total;
    delete transforms_total_font;
    delete transforms_total_font_ui;
  };
  void Reset() {
    fence_sync = 0;
    transforms_total -> Reset();
    transforms_total_font -> Reset();
    transforms_total_font_ui -> Reset();

    index_arena->Reset();
    vertex_arena->Reset();

    bound_buffers = 0;
  };

  // map buffers that updated asyncronously
  void MapBuffer();

  void WaitGPU(GLsync fence_sync,
               const source_location &location = source_location::current());
  void SyncGPUBufAndUnmap();
  void ActivateCommandBuffer();
  int BufferCommands(vector<DrawElementsIndirectCommand> &active_commands, int command_offset = -1);
  int BufferCommand(DrawElementsIndirectCommand &command, int command_offset = -1);
  void BufferMeshData(vector<MeshDataBase *> &load_data_meshes,
                      vector<shared_ptr<MeshLoadData>> &load_data);
  void BufferMeshData(MeshDataBase* mesh, shared_ptr<MeshLoadData> load_data);
  void BufferMeshData(MeshDataBase* mesh, shared_ptr<MeshLoadData> load_data,
                      vector<MeshDataBase *> &instances);
  DrawElementsIndirectCommand RequestStorage(MeshDataBase* mesh, unsigned long vertices_size, unsigned long indices_size);
  void BufferBoneTransform(unordered_map<unsigned int, mat4> &id_to_transform);
  void BufferBoneTransform(Bone *bone, mat4 &trans, unsigned int num_bones = 1);
  void BufferTransform(MeshData *mesh);
  void BufferTransform(vector<MeshData *> &mesh);
  void BufferLights(vector<Light *> &lights);
  void InitMeshBuffers();
  void BindVAOandBuffers(BufferType::BufferTypeFlag buffers_to_bind);

  void BufferTextureHandle(Texture* texture);
  void BufferMeshTexture(MeshData* mesh);
  void BufferFontTexture(MeshDataBase* mesh, Texture* texture);
  void Buffer3DFontTransform(MeshDataOverlay3D* mesh);;
  void BufferUniformDataUISize(MeshDataUI* mesh, int min_x, int max_x, int min_y, int max_y);
  void BufferUniformDataUITransform(MeshDataUI* mesh);;
  void BufferUniformDataController(int mouse_x, int mouse_y, int is_pressed, int is_click);

  void Dispose();

  void AddUsedBuffers(BufferType::BufferTypeFlag used_buffers);
 private:
  void PreDraw();
  void PostDraw();
  void _UnmapBuffer();
  CommandBuffer* _CreateCommandBuffer();
  void _DeleteCommandBuffer();
  long _GetTransformBucket(MeshData *mesh);
  long _GetTransformBucketFont(MeshDataOverlay3D *mesh);
  long _GetTransformBucketFontUI(MeshDataUI *mesh);
};
