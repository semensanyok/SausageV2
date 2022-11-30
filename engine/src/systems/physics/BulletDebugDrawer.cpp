#include "BulletDebugDrawer.h"

void BulletDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
    vec3 from3;
    vec3 to3;

    memcpy(&from3[0], &from[0], 3 * sizeof(float));
    memcpy(&to3[0], &to[0], 3 * sizeof(float));
    bool is_new_from = true;
    bool is_new_to = true;
    long index_from = -1;
    long index_to = -1;

    for (int i = 0; i < vertices.size(); i++) {
        if (vertices[i] == from3) {
            is_new_from = false;
            index_from = i;
        }
        if (vertices[i] == to3) {
            is_new_to = false;
            index_to = i;
        }
    }
    if (is_new_from || is_new_to) {
        vec3 color3;
        memcpy(&color3[0], &color[0], 3 * sizeof(float));
        if (is_new_from) {
            index_from = vertices.size();
            vertices.push_back(from3);
            colors.push_back(color3);
        }
        if (is_new_to) {
            index_to = vertices.size();
            vertices.push_back(to3);
            colors.push_back(color3);
        }
    }
    indices.push_back(index_from);
    indices.push_back(index_to);
};
void BulletDebugDrawer::drawLinePersist(const btVector3& from, const btVector3& to, const btVector3& color) {
    vec3 from3;
    vec3 to3;
    vec3 color3;
    memcpy(&color3[0], &color[0], 3 * sizeof(float));
    memcpy(&from3[0], &from[0], 3 * sizeof(float));
    memcpy(&to3[0], &to[0], 3 * sizeof(float));

    //lock_guard<mutex> data_lock(draw_call->command_ptr->buffer_lock->data_mutex);
    persist_draws.push_back({ state_manager->milliseconds_since_start + GameSettings::ray_debug_draw_lifetime_milliseconds, {from3, to3}, color3 });
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
        if (draw_call_manager->SetToCommandWithOffsets<MeshDataPhysDebugDrawer>(
          buffer->mesh, 1, false)) {
          draw_call_manager->physics_debug_dc->is_enabled = true;
        }
        clear();
    }
    else {
      Deactivate();
    }
};
