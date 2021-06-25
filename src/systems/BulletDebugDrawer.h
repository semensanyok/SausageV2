#pragma once

#include "sausage.h"
#include "Structures.h"
#include "Shader.h"
#include "BufferStorage.h"
#include "Renderer.h"

static struct PersistDrawRay {
    uint32_t remove_time_millis;
    vec3 vertices[2];
    vec3 color;
};
class BulletDebugDrawer : public btIDebugDraw
{

    MeshData* mesh_data;
    DrawCall* draw_call;
    Shader* debug_shader;
    BufferStorage* buffer;
    Renderer* renderer;
    int m_debugMode;

    vector<vec3> vertices;
    vector<unsigned int> indices;
    vector<vec3> colors;

    vector<PersistDrawRay> persist_draws;

    const unsigned int command_buffer_size = 1;
public:

    BulletDebugDrawer(Renderer* renderer, BufferStorage* buffer, Shader* debug_shader) : renderer{ renderer }, buffer{ buffer }, debug_shader{ debug_shader }, mesh_data{ nullptr } {
        draw_call = new DrawCall();
        draw_call->shader = debug_shader;
        draw_call->mode = GL_LINES;
        draw_call->buffer = buffer;
        draw_call->command_buffer = buffer->CreateCommandBuffer(command_buffer_size);
        renderer->AddDraw(draw_call);

        function<void()> buffer_debug_data_callback = std::bind(&BulletDebugDrawer::_BufferDataCallback, this);
        bool is_persistent_command = true;
        renderer->AddGlCommand(buffer_debug_data_callback, is_persistent_command);
    };
    ~BulletDebugDrawer() {
    };

    virtual void   drawLine(const btVector3& from, const btVector3& to, const btVector3& color);

    virtual void   drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);

    virtual void   reportErrorWarning(const char* warningString);

    virtual void   draw3dText(const btVector3& location, const char* textString);

    virtual void   setDebugMode(int debugMode);

    virtual int    getDebugMode() const { return m_debugMode; }

    void   drawLinePersist(const btVector3& from, const btVector3& to, const btVector3& color);

    void clearPersist();

private:
    void _BufferDataCallback() {
        if (vertices.size() > 0) {
            draw_call->command_count = 1;
            bool is_new_mesh_data = mesh_data == nullptr;
            vector<PersistDrawRay> keep_persist;
            for (auto& persist_draw : persist_draws) {
                indices.push_back(vertices.size());
                vertices.push_back(persist_draw.vertices[0]);
                indices.push_back(vertices.size());
                vertices.push_back(persist_draw.vertices[1]);
                colors.push_back(persist_draw.color);
                colors.push_back(persist_draw.color);
                if (GameSettings::milliseconds_since_start <= persist_draw.remove_time_millis) {
                    keep_persist.push_back(persist_draw);
                }
            }
            persist_draws = keep_persist;
            {
                shared_ptr<MeshLoadData> load_data = MeshManager::CreateMesh(vertices, indices, colors, is_new_mesh_data);
                if (is_new_mesh_data) {
                    mesh_data = load_data.get()->mesh_data;
                    mesh_data->name = "BulletDebugDrawerData";
                    // reserve last 1/4 of buffer for debug vertices;
                    mesh_data->vertex_offset = buffer->MAX_VERTEX - buffer->MAX_VERTEX * 0.25;
                    mesh_data->index_offset = buffer->MAX_INDEX - buffer->MAX_INDEX * 0.25;
                }
                else {
                    load_data.get()->mesh_data = mesh_data;
                }
                vector<shared_ptr<MeshLoadData>> vec_load_data = { load_data };
                bool is_transform_used = false;
                buffer->BufferMeshData(vec_load_data, is_transform_used);
            }
            int command_offset = buffer->AddCommand(mesh_data->command, draw_call->command_buffer, command_buffer_size);

            vertices.clear();
            indices.clear();
            colors.clear();
        }
        else {
            draw_call->command_count = 0;
        }
    }
};
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

    persist_draws.push_back({ GameSettings::milliseconds_since_start + GameSettings::ray_debug_draw_lifetime_milliseconds, {from3, to3}, color3 });
}
void BulletDebugDrawer::clearPersist() {
    persist_draws.clear();
}
void   BulletDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {
    vec3 from3;
    vec3 to3;
    vec3 color3;
    btVector3 to = PointOnB + normalOnB * distance;
    memcpy(&from3[0], &PointOnB[0], 3 * sizeof(float));
    memcpy(&to3[0], &to[0], 3 * sizeof(float));
    memcpy(&color3[0], &color[0], 3 * sizeof(float));

    indices.push_back(vertices.size());
    vertices.push_back(to3);
    colors.push_back(color3);
};
void   BulletDebugDrawer::reportErrorWarning(const char* warningString) {
};
void   BulletDebugDrawer::draw3dText(const btVector3& location, const char* textString) {
};
void   BulletDebugDrawer::setDebugMode(int debugMode) {
    m_debugMode = debugMode;
};
