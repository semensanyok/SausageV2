#include "MeshDataUI.h"
#include "BufferManager.h"
#include "DrawCall.h"

/**
* call once per frame for base mesh
*/

void MeshDataUI::FinalizeCommandWithBuffer() {
  DrawCall* dc = this->dc;
  dc->AddCommand(this->slots);
  BufferManager::GetInstance()->ui_buffer->BufferUniformOffset(this);
}

/**
* call once on first usage for new mesh
*/

bool MeshDataUI::AllocateUniformOffset() {
  if (IsUniformOffsetAllocated()) {
    return true;
  }
  return BufferManager::GetInstance()->ui_buffer->AllocateUniformOffset(this);
}

void MeshDataUI::OnTransformUpdate() {
  // buffer must be called each frame, because instance slot is new each time?
  // no. only instance slot update each frame.
  // transform update only if dirty
  if (Transform::is_dirty) {
    BufferManager::GetInstance()->ui_buffer->BufferTransform(this, ReadTransform());
  }
}
