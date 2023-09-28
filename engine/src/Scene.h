#pragma once

#include "sausage.h"
#include "BoundingVolume.h"
#include "Octree.h"
#include "SystemsManager.h"
#include "MeshDataTypes.h"

class Scene {
  vec3 world_extents;
  unsigned int octree_num_levels;
public:
  //Frustum* test_frustum = SausageDebug::GetCentered45DownFrustum();
  Frustum* test_frustum = new Frustum();
  SausageDebug::FrustumVertices test_frustum_verts;

  Scene(vec3 world_extents, unsigned int octree_num_levels) {
    auto sm = SystemsManager::GetInstance();
    sm->spatial_manager->Init(world_extents, octree_num_levels);
  }
  virtual void Init() {};
  virtual void PrepareFrameDraws() {
    auto sm = SystemsManager::GetInstance();
    sm->spatial_manager->CullPrepareDraws(
      // TODO: unset test Frustum
      // sm->camera->frustum
      test_frustum,
      sm->camera->pos);
  };
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

  void DebugSetAndDrawFrustum() {
    //auto scale = 1.0;
    auto scale = 0.1;
    //auto scale = 0.01;

    SausageDebug::GetScaledCameraFrustum(
      SystemsManager::GetInstance()->camera,
      test_frustum,
      scale);
    test_frustum_verts = SausageDebug::GetScaledCameraFrustumVertices(scale);

    auto sm = SystemsManager::GetInstance();
    auto load_data = sm->mesh_manager->CreateLoadData<VertexOutline>(
      test_frustum_verts.vertices,
      test_frustum_verts.indices);
    auto buf = sm->buffer_manager->mesh_outline_buffer;
    auto mesh = sm->mesh_manager->CreateMeshData<MeshDataOutline>();
    assert(buf->AllocateStorage(mesh->slots, load_data));
    mesh->slots.IncNumInstancesGetInstanceId();
    buf->BufferVertices(mesh->slots, load_data);
    
    sm->draw_call_manager->outline_dc->AddCommand(mesh->slots);
  }
};
