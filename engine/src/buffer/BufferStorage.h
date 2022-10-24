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
#include "OverlayStruct.h"

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
  //  TODO: each drawcall uses contigious range of commands. Need to allocate in advance for shader.
  //    or place shader with dynamic number of meshes at the end
  GLBuffers* gl_buffers;

public:

  static BufferStorage* GetInstance() {
    static BufferStorage* instance = new BufferStorage();
    return instance;
  };

  void Reset() {
    gl_buffers->Reset();
  };

  void Init();

  void BufferCommands(vector<DrawElementsIndirectCommand>& active_commands, int command_offset);
  void BufferCommand(DrawElementsIndirectCommand& command, int command_offset);
  void BufferMeshData(MeshDataBase* mesh, shared_ptr<MeshLoadData>& load_data);
  bool AllocateStorage(
    MeshDataSlots& out_slots,
    unsigned long vertices_size,
    unsigned long indices_size);
  bool AllocateInstanceSlot(
      MeshData& mesh,
      unsigned long num_instances);
  bool AllocateInstanceSlot(
      MeshDataUI& mesh,
      unsigned long num_instances);
  bool AllocateInstanceSlot(
      MeshDataOverlay3D& mesh,
      unsigned long num_instances);

  bool ReleaseStorage(MeshDataSlots& out_slots);
  void ReleaseInstanceSlot(MeshData& out_slots);
  void ReleaseInstanceSlot(MeshDataUI& out_slots);
  void ReleaseInstanceSlot(MeshDataOverlay3D& out_slots);

  void BufferBoneTransform(unordered_map<unsigned int, mat4>& id_to_transform);
  void BufferBoneTransform(Bone* bone, mat4& trans, unsigned int num_bones = 1);
  void BufferLights(vector<Light*>& lights);

  template<typename TRANSFORM_TYPE, typename MESH_TYPE>
  void BufferTransform(MESH_TYPE& mesh, TRANSFORM_TYPE& transform);

  /**
   * used by TextureManager, should not anywhere else
  */
  Texture* CreateTextureWithBufferSlot(GLuint gl_texture_id, GLuint64 gl_texture_handle_ARB);
  bool ReleaseTexture(Texture* texture) {
    gl_buffers->
      texture_handle_by_texture_id_ptr.instances_slots->ReleaseNumber(texture->id);
    texture->Dispose();
  };
  void BufferUniformDataUISize(MeshDataUI* mesh, int min_x, int max_x, int min_y, int max_y);
  void BufferUniformDataController(int mouse_x, int mouse_y, int is_pressed, int is_click);

  void AddUsedBuffers(BufferType::BufferTypeFlag used_buffers);
  void PreDraw();
  void PostDraw();
private:
  void BufferTextureHandle(Texture* texture);
  bool _AllocateInstanceSlot(MeshDataBase& mesh,
    InstancesSlots& buffer_instances_slots,
    unsigned int* base_instance_offset_ptr,
    unsigned long num_instances);
  BufferStorage() :
    gl_buffers{ new GLBuffers() },
    command_slots{ new ThreadSafeNumberPool(MAX_BASE_MESHES) },
    buffer_instances_slots{ new Arena({0,MAX_BASE_AND_INSTANCED_MESHES}) },
    transforms_font_slots{ new ThreadSafeNumberPool(MAX_3D_OVERLAY_TRANSFORM) },
    transforms_font_ui_slots{ new ThreadSafeNumberPool(MAX_UI_UNIFORM_TRANSFORM) },
    /**
    * had idea to have multiple arenas for each storage
    * to store large objects in one, smaller in other
    * now for simplicity - keep 1 Arena per buffer(with offset = 0)
    *
    * is_allocate_powers_of_2 = true
    * because lots of alloc/dealloc of multipurposed out_slots of various sizes
    **/
    index_arena{ new Arena({ 0, MAX_INDEX }, true) },
    vertex_arena{ new Arena({ 0, MAX_VERTEX }, true) }
  {};
  ~BufferStorage() {
    delete buffer_instances_slots;
    delete transforms_font_slots;
    delete transforms_font_ui_slots;
    delete gl_buffers;
  };
};
