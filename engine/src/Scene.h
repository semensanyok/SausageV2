#pragma once

#include "sausage.h"
#include "BoundingVolume.h"
#include "Octree.h"

class Scene {
public:
  Scene(vec3 world_extents, unsigned int octree_num_levels) {
  }
  virtual void Init() {};
  virtual void PrepareFrameDraws() {};
  void DebugDrawOctree() {
    //vector<vec3> out_vert;
    //vector<unsigned int> out_ind;
    //vector<vec3> out_colors;
    {
      auto sm = SystemsManager::GetInstance();
      sm->CreateDebugDrawer();
      AddVertices(
        sm->bullet_debug_drawer,
        sm->spatial_manager->scene_tree
        //,
        //out_vert,
        //out_ind,
        //out_colors
      );
      //auto load_data = sm->mesh_manager->CreateLoadData<VertexOutline>(out_vert, out_ind, out_colors);
      //auto mesh = sm->mesh_manager->CreateMeshData<MeshDataOutline>();
      //sm-> ->->AllocateStorage<VertexOutline>(
      //  mesh->slots, out_vert.size(), out_ind.size());
      //sm->buffer_manager->vertex_attributes->BufferVertices<VertexOutline>(mesh->slots, load_data);

      //auto dcm = sm->draw_call_manager;
      //auto debug_octree_dc = dcm->CreateDrawCall(
      //sm->shader_manager->all_shaders->outline,
      //  GL_LINES,
      //  sm->buffer_manager->command_buffer_manager->command_buffers.outline,
      //  true
      //);
      //sm->draw_call_manager->AddNewCommandToDrawCall<MeshDataOutline>(mesh,
      //  debug_octree_dc, 1, false);
      //sm->renderer->AddDraw(debug_octree_dc, DrawOrder::OUTLINE);
    }
  }

  void AddVertices(
    BulletDebugDrawer* drawer,
    Octree* node
    //,
    //vector<vec3>& out_vert,
    //vector<unsigned int>& out_ind,
    //vector<vec3>& out_colors
  ) {
    auto bv = node->bv;
    auto ext = node->bv->half_extents;
    auto color = vec3(255, 255, 0);
    vector<vec3> box_verts = {
      bv->center + vec3(-ext.x, -ext.y, -ext.z),
      bv->center + vec3(-ext.x, -ext.y, ext.z),
      bv->center + vec3(ext.x, -ext.y, ext.z),
      bv->center + vec3(ext.x, -ext.y, -ext.z),

      bv->center + vec3(-ext.x, ext.y, -ext.z),
      bv->center + vec3(-ext.x, ext.y, ext.z),
      bv->center + vec3(ext.x, ext.y, ext.z),
      bv->center + vec3(ext.x, ext.y, -ext.z),
    };

    vector<pair<int, int>> edges = {
      {0, 1},
      {1, 2},
      {2, 3},
      {3, 0},
      {0, 0 + 4},
      {1, 1 + 4},
      {2, 2 + 4},
      {3, 3 + 4},
      {0 + 4, 1 + 4},
      {1 + 4, 2 + 4},
      {2 + 4, 3 + 4},
      {3 + 4, 0 + 4},
    };
    for (auto& edge : edges) {
      drawer->drawLinePersist(box_verts[edge.first], box_verts[edge.second], color,
        UINT32_MAX);
    }
      //out_vert.push_back(box_verts[i]);
      //out_colors.push_back(vec3(255,0,0));
      //out_ind.push_back(i);

      for (auto child : node->children) {
      AddVertices(drawer, child
        //, out_vert, out_ind, out_colors
      );
    }
  }
};
