#include "BulletDebugDrawer.h"

void BulletDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
  vec3 from3;
  vec3 to3;
  vec3 color3;

  memcpy(&from3[0], &from[0], 3 * sizeof(float));
  memcpy(&to3[0], &to[0], 3 * sizeof(float));
  memcpy(&color3[0], &color[0], 3 * sizeof(float));

  drawLine(from3, to3, color3);
}
void BulletDebugDrawer::drawLine(const vec3& from, const vec3& to, const vec3& color)
{
  //lock_guard<mutex> data_lock(spatial_manager->outline_dc->command_buffer->buffer_lock->data_mutex);

  long index_from = -1;
  long index_to = -1;
  //if (is_new_from) {
  index_from = vertices.size();
  vertices.push_back(from);
  colors.push_back(color);
  //}
  //if (is_new_to) {
  index_to = vertices.size();
  vertices.push_back(to);
  colors.push_back(color);
  //}
  //}
  indices.push_back(index_from);
  indices.push_back(index_to);
}

void BulletDebugDrawer::drawLinePersist(const btVector3& from, const btVector3& to, const btVector3& color,
  uint32_t life_time_ms) {
  vec3 from3;
  vec3 to3;
  vec3 color3;
  memcpy(&color3[0], &color[0], 3 * sizeof(float));
  memcpy(&from3[0], &from[0], 3 * sizeof(float));
  memcpy(&to3[0], &to[0], 3 * sizeof(float));

  drawLinePersist(from3, to3, color3, life_time_ms);
}

void BulletDebugDrawer::drawLinePersist(const vec3& from, const vec3& to, const vec3& color,
  uint32_t life_time_ms)
{
  //lock_guard<mutex> data_lock(spatial_manager->outline_dc->command_buffer->buffer_lock->data_mutex);

  persist_draws.push_back({ life_time_ms == UINT32_MAX ?
    life_time_ms : state_manager->milliseconds_since_start + life_time_ms, {from, to}, color });
}

void   BulletDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {
  vec3 from3;
  vec3 to3;
  vec3 color3;
  btVector3 to = PointOnB + normalOnB * distance;
  memcpy(&from3[0], &PointOnB[0], 3 * sizeof(float));
  memcpy(&to3[0], &to[0], 3 * sizeof(float));
  memcpy(&color3[0], &color[0], 3 * sizeof(float));

  //lock_guard<mutex> data_lock(data_mutex);
  indices.push_back(vertices.size());
  vertices.push_back(to3);
  colors.push_back(color3);
};
void BulletDebugDrawer::flushLines() {
  Activate();
  if (vertices.size() > 0) {
    vector<PersistDrawRay> keep_persist;
    for (auto& persist_draw : persist_draws) {
      indices.push_back(vertices.size());
      vertices.push_back(persist_draw.vertices[0]);
      indices.push_back(vertices.size());
      vertices.push_back(persist_draw.vertices[1]);
      colors.push_back(persist_draw.color);
      colors.push_back(persist_draw.color);
      if (state_manager->milliseconds_since_start <= persist_draw.remove_time_millis) {
        keep_persist.push_back(persist_draw);
      }
    }
    persist_draws = keep_persist;
    buffer->BufferMeshData(vertices, indices, colors);
    buffer->mesh->dc->PutCommand(buffer->mesh->slots, 0);
    clear();
  }
  else {
    Deactivate();
  }
};
