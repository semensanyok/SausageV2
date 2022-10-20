#pragma once

#include "sausage.h"
#include "Macros.h"
#include "Settings.h"
#include "Arena.h"
#include "ThreadSafeNumberPool.h"
#include "OpenGLHelpers.h"
#include "Logging.h"
#include "GPUUniformsStruct.h"
#include "AnimationStruct.h"
#include "LightStruct.h"
#include "GLBuffers.h"
#include "TextureStruct.h"
#include "Texture.h"
#include "MeshDataStruct.h"

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

class BufferStorage {

 private:

  ThreadSafeNumberPool* command_slots;
  // instanced meshes must have contigious chunk of transform matrices to refer by base id + instance id
  Arena* instances_slots;
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
    instances_slots-> Reset();
    transforms_font_slots -> Reset();
    transforms_font_ui_slots -> Reset();

    index_arena->Reset();
    vertex_arena->Reset();

    gl_buffers->Reset();
  };

  void Init();

  void BufferCommands(vector<DrawElementsIndirectCommand> &active_commands, int command_offset);
  void BufferCommand(DrawElementsIndirectCommand &command, int command_offset);
  void BufferMeshData(MeshDataBase* mesh, shared_ptr<MeshLoadData>& load_data);
  bool AllocateStorage(
    MeshDataSlots& out_slots,
    unsigned long vertices_size,
    unsigned long indices_size,
    unsigned long num_instances);
  void ReleaseStorage(MeshDataSlots& out_slots);
  bool AllocateInstancesSlot(
    MeshDataSlots& out_slots,
    unsigned long num_instances);
  void ReleaseInstancesSlot(MeshDataSlots& out_slots);
  void BufferBoneTransform(unordered_map<unsigned int, mat4> &id_to_transform);
  void BufferBoneTransform(Bone *bone, mat4 &trans, unsigned int num_bones = 1);
  /**
   * @brief instance_offset is base mesh offset (MeshDataSlots#instances_slot#offset)
   *        plus instance id (glInstanceID), 0 for base mesh
  */
  void BufferTransform(BufferInstanceOffset* offset, mat4& transform);
  void BufferLights(vector<Light *> &lights);
  void BindVAOandBuffers(BufferType::BufferTypeFlag buffers_to_bind);

  void BufferTextureHandle(Texture* texture);
  void BufferTexture(BufferInstanceOffset* offset, Texture* texture);
  void BufferUniformDataUISize(MeshDataUI* mesh, int min_x, int max_x, int min_y, int max_y);
  void BufferUniformDataUITransform(MeshDataUI* mesh);;
  void BufferUniformDataController(int mouse_x, int mouse_y, int is_pressed, int is_click);

  void AddUsedBuffers(BufferType::BufferTypeFlag used_buffers);
  void PreDraw();
  void PostDraw();
 private:
   BufferStorage() :
     gl_buffers { new GLBuffers() },
     command_slots { new ThreadSafeNumberPool(MAX_BASE_MESHES) },
     instances_slots{ new Arena({0,MAX_BASE_AND_INSTANCED_MESHES}) },
     transforms_font_slots { new ThreadSafeNumberPool(MAX_3D_OVERLAY_TRANSFORM) },
     transforms_font_ui_slots { new ThreadSafeNumberPool(MAX_UI_UNIFORM_TRANSFORM) },
     // TODO:
     //  - each drawcall uses contigious range of commands. Need to allocate in advance for shader.
     //    or place shader with dynamic number of meshes at the end

     /**
     * had idea to have multiple arenas for each storage
     * to store large objects in one, smaller in other
     * now for simplicity - keep 1 Arena per buffer(with offset = 0)
     **/
     index_arena { new Arena({ 0, MAX_INDEX }) },
     vertex_arena { new Arena({ 0, MAX_VERTEX }) }
   {};
   ~BufferStorage() {
     delete instances_slots;
     delete transforms_font_slots;
     delete transforms_font_ui_slots;
     delete gl_buffers;
   };
};
