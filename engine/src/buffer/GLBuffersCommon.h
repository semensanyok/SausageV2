#pragma once

#include "sausage.h"
#include "Arena.h"
#include "InstancesSlots.h"

using namespace std;

namespace GLBuffersCommon {
  const GLbitfield flags =
    GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
}

using namespace GLBuffersCommon;

namespace BufferType {
  typedef unsigned int BufferTypeFlag;
  const BufferTypeFlag MESH_UNIFORMS = 1;
  const BufferTypeFlag TEXTURE = 1 << 2;
  const BufferTypeFlag LIGHT = 1 << 3;
  const BufferTypeFlag UI_UNIFORMS = 1 << 4;
  const BufferTypeFlag CONTROLLER_UNIFORMS = 1 << 5;
  const BufferTypeFlag MESH_STATIC_UNIFORMS = 1 << 6;
  const BufferTypeFlag MESH_TERRAIN_UNIFORMS = 1 << 7;

  // COMPOSITE FLAGS
  const BufferTypeFlag MESH_BUFFERS =
    MESH_UNIFORMS | TEXTURE | LIGHT;
  const BufferTypeFlag MESH_STATIC_BUFFERS =
    MESH_STATIC_UNIFORMS | TEXTURE | LIGHT;
  const BufferTypeFlag MESH_TERRAIN_BUFFERS =
    MESH_TERRAIN_UNIFORMS | TEXTURE | LIGHT;
  const BufferTypeFlag OUTLINE_BUFFERS =
    0;
  const BufferTypeFlag UI_BUFFERS =
    TEXTURE | UI_UNIFORMS | CONTROLLER_UNIFORMS;
};  // namespace BufferType

inline void DeleteBuffer(GLenum target, GLuint buffer_id) {
  glBindBuffer(target, buffer_id);
  glUnmapBuffer(target);
  glDeleteBuffers(1, &buffer_id);
  DEBUG_EXPR(CheckGLError());
}

template<typename T>
BufferSlots<T>* CreateBufferSlots(unsigned long storage_size,
  unsigned long max_arena_slots,
  GLuint array_type,
  ArenaSlotSize slot_size) {
  GLuint buffer_id;
  glGenBuffers(1, &buffer_id);
  glBindBuffer(array_type, buffer_id);
  glBufferStorage(array_type, storage_size, NULL, flags);
  T* buffer_ptr = (T*)glMapBufferRange(array_type, 0,
    storage_size, flags);
  // TODO: test correctness and adjust if necessary
  return new BufferSlots<T>{ {Arena({ 0, max_arena_slots, 0}, slot_size)}, buffer_id , buffer_ptr };
}

template<typename T>
BufferNumberPool<T>* CreateBufferNumberPool(unsigned long storage_size,
  unsigned long max_number,
  GLuint array_type) {
  GLuint buffer_id;
  glGenBuffers(1, &buffer_id);
  glBindBuffer(array_type, buffer_id);
  glBufferStorage(array_type, storage_size, NULL, flags);
  T* buffer_ptr = (T*)glMapBufferRange(array_type, 0,
    storage_size, flags);
  return new BufferNumberPool<T>{ ThreadSafeNumberPool(max_number), buffer_id , buffer_ptr };
}
