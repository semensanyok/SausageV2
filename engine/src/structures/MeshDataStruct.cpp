#include "MeshDataStruct.h"
#include "DrawCall.h"

void MeshDataBase::FinalizeCommandWithBuffer() {
  if (!this->slots.IsBufferIdAllocated()) {
    this->dc->AddCommand(this->slots);
  }
}
