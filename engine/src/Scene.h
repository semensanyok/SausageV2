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
  // TODO: unset test Frustum
  Frustum* test_frustum = SausageDebug::GetCentered45DownFrustum();

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
      test_frustum, sm->camera->pos);
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

  void DebugDrawFrustum(Frustum* fr) {
    auto c = SystemsManager::GetInstance()->camera;
    // other than 1 for demonstration
    //const float scale = 1.0;
    const float scale = 0.1;
    const float half_far_width = c->far_plane * scale * tanf(c->FOV_rad * .5f);
    const float half_far_height = half_far_width * scale * c->aspect;
    const glm::vec3 far_center = c->far_plane * scale * c->direction;

    const float half_near_width = c->near_plane * scale * tanf(c->FOV_rad * .5f);
    const float half_near_height = half_near_width * scale * c->aspect;
    const glm::vec3 near_center = c->near_plane * scale * c->direction;

    auto sm = SystemsManager::GetInstance();
    sm->CreateDebugDrawer();
    vector<vec3> box_verts = {
      near_center - c->right * half_near_width + c->up * half_near_height,
      near_center + c->right * half_near_width + c->up * half_near_height,
      near_center - c->right * half_near_width - c->up * half_near_height,
      near_center + c->right * half_near_width - c->up * half_near_height,

      far_center - c->right * half_far_width + c->up * half_far_height,
      far_center + c->right * half_far_width + c->up * half_far_height,
      far_center - c->right * half_far_width - c->up * half_far_height,
      far_center + c->right * half_far_width - c->up * half_far_height,
    };
    auto color = vec3(255, 0, 0);

    // GL_LINES: Vertices 0 and 1 are considered a line. Vertices 2 and 3 are considered a line. And so on. If the user specifies a non-even number of vertices, then the extra vertex is ignored.
    vector<unsigned int> indices = {
      0, 1,
      1, 3,
      3, 2,
      2, 0,

      0, 4,
      1, 5,
      2, 6,
      3, 7,

      4, 5,
      5, 7,
      7, 6,
      6, 4
    };

    auto load_data = sm->mesh_manager->CreateLoadData<VertexOutline>(box_verts, indices);
    auto buf = sm->buffer_manager->mesh_outline_buffer;
    auto mesh = sm->mesh_manager->CreateMeshData<MeshDataOutline>();
    assert(buf->AllocateStorage(mesh->slots, load_data));
    mesh->slots.IncNumInstancesGetInstanceId();
    buf->BufferVertices(mesh->slots, load_data);
    
    sm->draw_call_manager->outline_dc->AddCommand(mesh->slots);
  }
};
