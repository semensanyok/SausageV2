#pragma once

#include "sausage.h"
#include "Structures.h"
#include "ShaderStruct.h"
#include "Shader.h"
#include "Renderer.h"
#include "Settings.h"
#include "StateManager.h"
#include "BulletDebugDrawerBufferConsumer.h"
#include "Renderer.h"

using namespace std;

static struct PersistDrawRay {
    uint32_t remove_time_millis;
    vec3 vertices[2];
    vec3 color;
};
class BulletDebugDrawer : public btIDebugDraw
{
    DrawCall* draw_call;
    Shader* debug_shader;
    Renderer* renderer;
    BulletDebugDrawerBufferConsumer* buffer;
    StateManager* state_manager;
    int m_debugMode;

    vector<vec3> vertices;
    vector<unsigned int> indices;
    vector<vec3> colors;

    vector<PersistDrawRay> persist_draws;

    const unsigned int command_buffer_size = 1;
public:
    BulletDebugDrawer(Renderer* renderer,
        BulletDebugDrawerBufferConsumer* buffer,
        Shaders* shaders,
        StateManager* state_manager) :
        renderer{ renderer },
        buffer{ buffer },
        debug_shader{ shaders->bullet_debug },
      state_manager{ state_manager } {
      draw_call = buffer->CreateDrawCall(shaders->bullet_debug, buffer->CreateCommandBuffer(command_buffer_size), GL_LINES);
      draw_call->command_count = 1;
      Activate();
      CheckGLError();
    };
    ~BulletDebugDrawer() {
        Deactivate();
    };

    void   drawLine(const btVector3& from, const btVector3& to, const btVector3& color);

    void   drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);

    void   reportErrorWarning(const char* warningString) {
    }

    void   draw3dText(const btVector3& location, const char* textString) {
    }

    void   setDebugMode(int debugMode) {
        m_debugMode = debugMode;
    }

    int    getDebugMode() const { return m_debugMode; }

    void flushLines();
    
    void Activate() {
      renderer->AddDraw(draw_call, DrawOrder::MESH);
      draw_call->buffer->ActivateCommandBuffer(draw_call->command_buffer);
    }
    void Deactivate() {
        clearPersist();
        clear();
        renderer->RemoveDraw(draw_call, DrawOrder::MESH);
        draw_call->buffer->RemoveCommandBuffer(draw_call->command_buffer);
    }
    void   drawLinePersist(const btVector3& from, const btVector3& to, const btVector3& color);
private: 
    void clearPersist() {
        persist_draws.clear();
    };
    void clear() {
        vertices.clear();
        indices.clear();
        colors.clear();
    }
};
