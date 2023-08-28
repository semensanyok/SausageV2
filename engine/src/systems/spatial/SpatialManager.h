#pragma once

#include "sausage.h"
#include "MeshDataStruct.h"
#include "Spatial.h"
#include "DrawCall.h"
#include "DrawCallManager.h"
#include "Octree.h"
#include "Frustum.h"
#include "Transform.h"
#include "InstanceSlot.h"
#include "BufferManager.h"

using namespace std;
using namespace glm;

class SpatialManager {
  DrawCallManager* dc_manager;
  BufferManager* buffer_manager;

public:
  SpatialManager(DrawCallManager* dc_manager,
    BufferManager* buffer_manager) :
    dc_manager{ dc_manager },
    buffer_manager{ buffer_manager } {}
  Octree* scene_tree;

  void Init(vec3 world_extents, unsigned int octree_num_levels) {
    auto center = vec3(0, 0, 0);
    scene_tree = new Octree(new BoundingBox(center, world_extents, false), octree_num_levels);
  };

  void CullPrepareDraws(Frustum* frustum, vec3& camera_pos) {
    vector<Spatial*> out_inside_frustum;

    // transform mustf be buffered after buffer_id and instance_id was set per instance
    vector<InstanceSlotUpdate*> isntance_slot_updates;
    vector<Transform<mat4>*> transform_updates;

    scene_tree->FrustrumCull(frustum, camera_pos, out_inside_frustum);
    for (auto mesh : out_inside_frustum) {
      // TODO: exhaust somehow. audio/particles/other spatial effects
      Transform<mat4>* trans_update_mesh = dynamic_cast<Transform<mat4>*>(mesh);
      if (trans_update_mesh != nullptr) {
        transform_updates.push_back(trans_update_mesh);
      }
      InstanceSlotUpdate* instance_slot_update = dynamic_cast<InstanceSlotUpdate*>(mesh);
      if (instance_slot_update != nullptr) {
        isntance_slot_updates.push_back(instance_slot_update);
        continue;
      }
      
      // TODO: buffer transform requires cur frame alocated GetInstanceOffset():
      //  1: assign base mesh instance offset
      //     for each instance - assign instance_id
      //  2. add command. (final num instances for each base mesh was calculated in 1)
      //  3: buffer transform
      //  4: unset frame offsets
      //  5: reset draw calls command buffers
    }
    //  1: assign base mesh instance offset
    for (InstanceSlotUpdate* upd : isntance_slot_updates) {
      upd->AllocateUniformOffset();
      upd->IncNumInstancesSetInstanceId();
    }
    // 2. add command
    for (InstanceSlotUpdate* upd : isntance_slot_updates) {
      upd->FinalizeCommandWithBuffer();
    }
    //  3: buffer transform
    for (Transform<mat4>* upd : transform_updates) {
      upd->OnTransformUpdate();
    }
    // 4: invalidate instance slot for next frame (optional?)
    for (InstanceSlotUpdate* upd : isntance_slot_updates) {
      upd->UnsetFrameOffsets();
    }

  }

  //template<typename MESH_TYPE>
  //void InsertToOctree(MESH_TYPE* mesh) {
  //  if (!scene_tree->Insert(mesh)) {
  //    LOG(format("Failed to scene_tree->Insert(mesh) for %s", mesh->name));
  //  }
  //}

  template<typename MESH_TYPE, typename UNIFORM_TYPE>
  void InsertToOctree(MeshDataInstance<MESH_TYPE, UNIFORM_TYPE>* mesh) {
    if (!scene_tree->Insert(mesh)) {
      LOG(format("Failed to scene_tree->Insert(mesh) for %s", mesh->name));
    }
  }

  void PostDraw() {
    // reset draw calls, populated during culling
    dc_manager->mesh_dc->Reset();
    dc_manager->mesh_static_dc->Reset();
    dc_manager->terrain_dc->Reset();
  }
};
