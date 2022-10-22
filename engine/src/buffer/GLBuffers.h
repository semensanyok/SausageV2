#pragma once

#include "sausage.h"
#include "GPUStructs.h"
#include "GPUUniformsStruct.h"
#include "Vertex.h"

using namespace std;
using namespace UniformsLocations;
using namespace BufferSizes;

template<typename T>
struct UniformOffsets {
  // must be allocated with size = MAX_INSTANCES
  // index to uniform data as offset + glInstanceId
  Arena* instances_slots;
  T* uniform_ptr;
};

class GLBuffers {
  ///////////
  /// Buffers
  ///////////
  GLuint mesh_VAO;
  GLuint vertex_buffer;
  GLuint index_buffer;
  // UNIFORMS AND SSBO
  GLuint uniforms_buffer;
  //GLuint blend_textures_by_mesh_id_buffer;
  GLuint texture_handle_by_texture_id_buffer;
  GLuint light_buffer;
  GLuint font_texture_buffer;
  GLuint uniforms_3d_overlay_buffer;
  GLuint uniforms_ui_buffer;
  GLuint uniforms_controller_buffer;

  BufferType::BufferTypeFlag used_buffers;
  BufferType::BufferTypeFlag bound_buffers;

  const GLbitfield flags =
    GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
  GLsync fence_sync = 0;
  bool is_need_barrier = false;
public:
  //////////////////////////
  // Mapped buffers pointers
  //////////////////////////
  Vertex* vertex_ptr;
  unsigned int* index_ptr;

  CommandBuffer* command_buffer;
  UniformOffsets<MeshUniform> uniforms_ptr;
  //BlendTexturesMeshUniform* blend_textures_mesh_ptr;
  LightsUniform* light_ptr;
  GLuint64* texture_handle_by_texture_id_ptr;
  UniformOffsets<UniformData3DOverlay> uniforms_3d_overlay_ptr;
  UniformOffsets<UniformDataUI> uniforms_ui_ptr;
  UniformOffsets<ControllerUniformData> uniforms_controller_ptr;

  // call after SSBO write (GL_SHADER_STORAGE_BUFFER).
  void SetSyncBarrier() {
    is_need_barrier = true;
  }
  void Reset() {
    fence_sync = 0;
    used_buffers = 0;
  };
  void InitBuffers();
  void BindVAOandBuffers();
  void Dispose();
  void AddUsedBuffers(BufferType::BufferTypeFlag used_buffers);
  void MapBuffer();
  void PreDraw();
  void PostDraw();
private:
  void _SyncGPUBufAndUnmap();
  void _BindCommandBuffer();
  void _UnmapBuffer();
  CommandBuffer* _CreateCommandBuffer();
  void _DeleteCommandBuffer(CommandBuffer* command_buffer) {
    lock_guard<mutex> data_lock(command_buffer->buffer_lock->data_mutex);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffer->id);
    glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
    DEBUG_EXPR(CheckGLError());
    delete command_buffer;
  }
  void WaitGPU(GLsync fence_sync,
               const source_location& location = source_location::current());
};
