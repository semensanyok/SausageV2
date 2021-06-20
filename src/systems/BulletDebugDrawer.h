#pragma once

#include "sausage.h"
#include "Structures.h"
#include "Shader.h"
#include "BufferStorage.h"
#include "Renderer.h"


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

public:

    BulletDebugDrawer(Renderer* renderer, BufferStorage* buffer, Shader* debug_shader) : renderer{ renderer }, buffer{ buffer }, debug_shader{ debug_shader }, mesh_data{ nullptr } {
        draw_call = new DrawCall(GL_LINE_STRIP, buffer, debug_shader, 1, 0, 0);
        function<void()> callback = std::bind(&BulletDebugDrawer::_BufferDataCallback, this);
        renderer->AddGlCommand(callback);
    };
    ~BulletDebugDrawer() {
    };

    virtual void   drawLine(const btVector3& from, const btVector3& to, const btVector3& color);

    virtual void   drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);

    virtual void   reportErrorWarning(const char* warningString);

    virtual void   draw3dText(const btVector3& location, const char* textString);

    virtual void   setDebugMode(int debugMode);

    virtual int      getDebugMode() const { return m_debugMode; }

private:
    void _BufferDataCallback() {
        if (vertices.size() > 0) {
            bool is_new_mesh_data = mesh_data == nullptr;
            {
                shared_ptr<MeshLoadData> load_data = MeshManager::CreateMesh(vertices, indices, is_new_mesh_data);
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
                buffer->BufferMeshData(vec_load_data);
            }
            int command_offset = buffer->AddCommand(mesh_data->command, draw_call->command_offset);
            draw_call->command_offset = command_offset;

            vertices.clear();
            indices.clear();
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
    if (is_new_from) {
        index_from = vertices.size();
        vertices.push_back(from3);
    }
    if (is_new_to) {
        index_to = vertices.size();
        vertices.push_back(to3);
    }
    indices.push_back(index_from);
    indices.push_back(index_to);
};
void   BulletDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {
};
void   BulletDebugDrawer::reportErrorWarning(const char* warningString) {
};
void   BulletDebugDrawer::draw3dText(const btVector3& location, const char* textString) {
};
void   BulletDebugDrawer::setDebugMode(int debugMode) {
};
