#pragma once

#include "sausage.h"
#include "Structures.h"
#include "ShaderStruct.h"
#include "Shader.h"
#include "Renderer.h"
#include "Settings.h"
#include "StateManager.h"
#include "BufferManager.h"
#include "DrawCallManager.h"

using namespace std;
using namespace glm;

static struct PersistDrawRay {
  uint32_t remove_time_millis;
  vec3 vertices[2];
  vec3 color;
};
class BulletDebugDrawer : public btIDebugDraw
{
  DrawCallManager* draw_call_manager;
  BulletDebugDrawerBufferConsumer* buffer;
  StateManager* state_manager;
  int m_debugMode;

  vector<vec3> vertices;
  vector<unsigned int> indices;
  vector<vec3> colors;

  vector<PersistDrawRay> persist_draws;

  const unsigned int command_buffer_size = 1;

  bool is_called_AddNewCommandToDrawCall = false;
public:
  BulletDebugDrawer(
      BufferManager* buffer_manager,
      DrawCallManager* draw_call_manager,
      StateManager* state_manager) :
    buffer{ buffer_manager->GetPhysDebugDrawer() },
    state_manager{ state_manager },
    draw_call_manager{ draw_call_manager }{
    // inited in buffer_manager->GetPhysDebugDrawer()
    //buffer->Init();

    //int debug_mask = btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawContactPoints;
    int debug_mask = btIDebugDraw::DBG_DrawWireframe;
    setDebugMode(debug_mask);
  };

  ~BulletDebugDrawer() {
    Deactivate();
  };

  // btIDebugDraw functions ////////////////////////////////////////////////////////////////////
  void   drawLine(const btVector3& from, const btVector3& to, const btVector3& color);

  void   drawLine(const vec3& from, const vec3& to, const vec3& color);

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

  void drawLinePersist(const btVector3& from, const btVector3& to, const btVector3& color,
    uint32_t life_time_ms = GameSettings::ray_debug_draw_lifetime_milliseconds);

  void drawLinePersist(const vec3& from, const vec3& to, const vec3& color,
    uint32_t life_time_ms = GameSettings::ray_debug_draw_lifetime_milliseconds);

  /////////////////////////////////////////////////////////////////////////////////////////////

  void Activate() {
    draw_call_manager->physics_debug_dc->is_enabled = true;
  }
  void Deactivate() {
    clearPersist();
    clear();
    draw_call_manager->physics_debug_dc->is_enabled = false;
  }
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
