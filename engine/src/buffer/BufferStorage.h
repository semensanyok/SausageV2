#pragma once

#include "sausage.h"
#include "GLBuffers.h"
#include "OpenGLHelpers.h"
#include "Settings.h"
#include "MeshDataStruct.h"
#include "AnimationStruct.h"
#include "GPUUniformsStruct.h"
#include "TextureStruct.h"
#include "LightStruct.h"
#include "Texture.h"
#include "Macros.h"
#include "ThreadSafeSet.h"
#include "ThreadSafeNumberPool.h"
#include "Arena.h"
#include "Logging.h"

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
  //friend class Renderer;

 private:

  ThreadSafeNumberPool* command_slots;
  ThreadSafeNumberPool* transforms_slots;
  ThreadSafeNumberPool* transforms_font_slots;
  ThreadSafeNumberPool* transforms_font_ui_slots;
  //  TODO: each drawcall uses contigious range of commands. Need to allocate in advance for shader.
  //    or place shader with dynamic number of meshes at the end
  Arena* index_arena;
  Arena* vertex_arena;
  GLBuffers* gl_buffers;

 public:

   static BufferStorage* GetInstance() {
     static BufferStorage* instance = new BufferStorage();
     return instance;
   };

  void Reset() {
    transforms_slots -> Reset();
    transforms_font_slots -> Reset();
    transforms_font_ui_slots -> Reset();

    index_arena->Reset();
    vertex_arena->Reset();

    gl_buffers->Reset();
  };

  void Init();

  void BufferCommands(vector<DrawElementsIndirectCommand> &active_commands, int command_offset);
  void BufferCommand(DrawElementsIndirectCommand &command, int command_offset);
  void BufferMeshData(MeshDataBase* mesh, shared_ptr<MeshLoadData> load_data);
  bool RequestBuffersOffsets(
    MeshDataBase* out_mesh,
    unsigned long vertices_size,
    unsigned long indices_size);
  void ReleaseStorage(MeshDataBase* mesh);
  void BufferBoneTransform(unordered_map<unsigned int, mat4> &id_to_transform);
  void BufferBoneTransform(Bone *bone, mat4 &trans, unsigned int num_bones = 1);
  void BufferTransform(MeshData *mesh);
  void BufferTransform(vector<MeshData *> &mesh);
  void BufferLights(vector<Light *> &lights);
  void BindVAOandBuffers(BufferType::BufferTypeFlag buffers_to_bind);

  void BufferTextureHandle(Texture* texture);
  void BufferMeshTexture(MeshData* mesh);
  void BufferFontTexture(MeshDataBase* mesh, Texture* texture);
  void Buffer3DFontTransform(MeshDataOverlay3D* mesh);;
  void BufferUniformDataUISize(MeshDataUI* mesh, int min_x, int max_x, int min_y, int max_y);
  void BufferUniformDataUITransform(MeshDataUI* mesh);;
  void BufferUniformDataController(int mouse_x, int mouse_y, int is_pressed, int is_click);

  void AddUsedBuffers(BufferType::BufferTypeFlag used_buffers);
  void PreDraw();
  void PostDraw();
 private:
   BufferStorage() {
     gl_buffers = new GLBuffers();
     command_slots = new ThreadSafeNumberPool(MAX_BASE_MESHES);
     transforms_slots = new ThreadSafeNumberPool(MAX_BASE_AND_INSTANCED_MESHES);
     transforms_font_slots = new ThreadSafeNumberPool(MAX_3D_OVERLAY_TRANSFORM);
     transforms_font_ui_slots = new ThreadSafeNumberPool(MAX_UI_UNIFORM_TRANSFORM);

     // TODO:
     //  - each drawcall uses contigious range of commands. Need to allocate in advance for shader.
     //    or place shader with dynamic number of meshes at the end

     /**
     * had idea to have multiple arenas for each storage
     * to store large objects in one, smaller in other
     * now for simplicity - keep 1 Arena per buffer(with offset = 0)
     **/
     unsigned int offset = 0;
     Arena* index_arena = new Arena({ MAX_INDEX, offset });
     Arena* vertex_arena = new Arena({ MAX_VERTEX, offset });
   };
   ~BufferStorage() {
     delete transforms_slots;
     delete transforms_font_slots;
     delete transforms_font_ui_slots;
     delete gl_buffers;
   };
};
