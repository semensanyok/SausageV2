#pragma once

#include "sausage.h"
#include "Structures.h"
#include "Interfaces.h"
#include "PhysicsStruct.h"
#include "BulletDebugDrawer.h"
#include "Settings.h"
#include "AssetUtils.h"
#include "StateManager.h"
#include "Macros.h"

using namespace std;

class PhysicsManager : public SausageSystem {
  btDefaultCollisionConfiguration* collisionConfiguration;
  btCollisionDispatcher* dispatcher;
  btBroadphaseInterface* overlappingPairCache;
  BulletDebugDrawer* debugDrawer;

  vector<btRigidBody*> rigidBodies;
  // TODO: reuse colision shapes. (from bullet docs)
  vector<btCollisionShape*> collisionShapes;

  btSequentialImpulseConstraintSolver* solver;
  btDiscreteDynamicsWorld* dynamicsWorld;

  StateManager* state_manager;
public:

  PhysicsManager(
    StateManager* state_manager,
    MeshDataBufferConsumer* mesh_buffer
  ) : state_manager{ state_manager } {
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    overlappingPairCache = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver();

    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, -9.8, 0));
    dynamicsWorld->setDebugDrawer(nullptr);
  }
  void SetDebugDrawer(BulletDebugDrawer* debugDrawer) {
    this->debugDrawer = debugDrawer;
    dynamicsWorld->setDebugDrawer(debugDrawer);
  }
  void Simulate() {
    IF_PROFILE_ENABLED(auto proft3 = chrono::steady_clock::now(););
    //dynamicsWorld->stepSimulation(state_manager->delta_time, 1);
    dynamicsWorld->stepSimulation(state_manager->delta_time * GameSettings::physics_step_multiplier, 1000);
    IF_PROFILE_ENABLED(ProfTime::physics_sym_ns = chrono::steady_clock::now() - proft3; auto proft8 = chrono::steady_clock::now(););
    dynamicsWorld->debugDrawWorld();
    IF_PROFILE_ENABLED(ProfTime::physics_debug_draw_world_ns = chrono::steady_clock::now() - proft8;);
  }
  void ClickRayTestClosest(float screen_x,
    float screen_y,
    const vec3& position,
    float distance,
    const mat4& camera_projection_view_inverse) {
    auto screen_x_normalized = (screen_x / GameSettings::SCR_WIDTH - 0.5f) * 2.0;
    auto screen_y_normalized = -((screen_y) / GameSettings::SCR_HEIGHT - 0.5f) * 2.0;
    vec4 world_end = camera_projection_view_inverse * vec4(screen_x_normalized, screen_y_normalized, 1, 1);
    world_end = world_end * distance;

    btVector3 btStart(position.x, position.y, position.z);
    btVector3 btEnd(world_end.x, world_end.y, world_end.z);

    btCollisionWorld::ClosestRayResultCallback rayCallback(
      btStart,
      btEnd
    );

    rayCallback.m_collisionFilterGroup =
      SausageCollisionMasks::CLICKABLE_GROUP_0;
    rayCallback.m_collisionFilterMask =
      SausageCollisionMasks::CLICKABLE_GROUP_0;

    dynamicsWorld->rayTest(
      btStart,
      btEnd,
      rayCallback
    );

    if (rayCallback.hasHit()) {
      auto sausage_up = (SausageUserPointer*)rayCallback.m_collisionObject->getUserPointer();
      if (dynamic_cast<MeshDataClickable*>(sausage_up)) {
        ((MeshDataClickable*)sausage_up)->Call();
      }
    }
    if (state_manager->phys_debug_draw) {
      debugDrawer->drawLinePersist(btStart, btEnd, { 255,0,0 });
    }
  }
  void UpdateTransforms() {
    IF_PROFILE_ENABLED(auto proft4 = chrono::steady_clock::now());
    auto nonStatic = dynamicsWorld->getNonStaticRigidBodies();
    btTransform btTrans;
    for (size_t i = 0; i < nonStatic.size(); i++)
    {
      auto rigidBody = nonStatic[i];
      rigidBody->getMotionState()->getWorldTransform(btTrans);
      // perhaps dynamic_cast will be required because of MeshDataClickable raytest
      PhysicsTransformUpdate* sausage_up = ((PhysicsTransformUpdate*)rigidBody->getUserPointer());
      
      //auto& update = state_manager->GetPhysicsUpdate(mesh_data);
      //update.first = mesh_data;
      //btTrans.getOpenGLMatrix(&mesh_data->transform[0][0]);

      btTrans.getOpenGLMatrix(&sausage_up->GetOutMatrix()[0][0]);
      sausage_up->OnTransformUpdate();
    }
    IF_PROFILE_ENABLED(ProfTime::physics_buf_trans_ns = chrono::steady_clock::now() - proft4);
  }
  void AddBoxRigidBody(PhysicsData* physics_data,
    PhysicsTransformUpdate* user_pointer,
    mat4& model_transform,
    const char* name_for_log) {
    vec3 half_extents = physics_data->max_AABB - physics_data->min_AABB;
    half_extents.x = half_extents.x / 2;
    half_extents.y = half_extents.y / 2;
    half_extents.z = half_extents.z / 2;

    btBoxShape* shape = new btBoxShape(btVector3(half_extents.x, half_extents.y, half_extents.z));
    collisionShapes.push_back(shape);
    auto transform = btTransform();
    if (model_transform[0].x > 1 || model_transform[1].y > 1 || model_transform[2].z > 1) {
      LOG((ostringstream()
        << "model scale must be 1 for bullet rigidbody. "
        "Incorrect collision prediction for mesh: "
        << name_for_log).str());
    }
    transform.setFromOpenGLMatrix(&model_transform[0][0]);
    btDefaultMotionState* motionstate = new btDefaultMotionState(transform);

    bool isDynamic = (physics_data->mass != 0.f);
    btVector3 localInertia(0, 0, 0);
    if (isDynamic) {
      shape->calculateLocalInertia(physics_data->mass,
                                   localInertia);
    }
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(
      physics_data->mass,  // mass, in kg. 0 -> Static object, will never move.
      motionstate,
      shape,  // collision shape of body
      localInertia    // local inertia
    );
    btRigidBody* rigidBody = new btRigidBody(rigidBodyCI);

    rigidBody->setUserPointer(user_pointer);
    dynamicsWorld->addRigidBody(rigidBody, physics_data->collision_group, physics_data->collides_with_groups);
    rigidBodies.push_back(rigidBody);
  }

  // TODO: continious collision for terrain
  // https://github.com/bulletphysics/bullet3/blob/master/src/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h
  /*
  * cites from btHeightfieldTerrainShape
  * 
  *   Most (but not all) rendering and heightfield libraries assume
  *   upAxis = 1 (that is, the y-axis is "up").
  *   This class allows any of the 3 coordinates to be "up".
  *   Make sure your choice of axis is consistent with your rendering system.
  */
  void AddTerrainRigidBody(
    //PhysicsData* physics_data,
    vector<float>& heightmap,
    int width,
    int height,
    btScalar spacing,
    SausageUserPointer* user_pointer,
    mat4& model_transform,
    string& name_for_log,
    btScalar minHeight,
    btScalar maxHeight
  ) {
    bool flipQuadEdges = false;

    // legacy constructor
    //btScalar s_gridHeightScale = 0.02;
    //btHeightfieldTerrainShape* terrain_shape = new btHeightfieldTerrainShape(
    //  width, height, heightmap.data(), s_gridHeightScale, 0.0, 1.0,
    //  1, PHY_FLOAT, flipQuadEdges);

    // Y axis
    int up_axis = 1;
    btHeightfieldTerrainShape* shape = new btHeightfieldTerrainShape(
      width, height, heightmap.data(), minHeight, maxHeight,
      up_axis,
      flipQuadEdges
    );
    //spacing *= 100;
    btVector3 localScaling = { spacing, spacing, spacing };
    localScaling[up_axis] = 1.0;
    shape->setLocalScaling(localScaling);

    //if (m_upAxis == 2)
      //shape->setFlipTriangleWinding(true);

    // create ground object
    float mass = 0.0;
    auto transform = btTransform();
    if (model_transform[0].x > 1 || model_transform[1].y > 1 || model_transform[2].z > 1) {
      LOG((ostringstream()
        << "model scale must be 1 for bullet rigidbody. "
        "Incorrect collision prediction for mesh: "
        << name_for_log).str());
    }
    {
      transform.setFromOpenGLMatrix(&model_transform[0][0]);
      // SAUSAGE sets plain origin to southwest, not center
      // bullet expects origin to be at the center
      //  auto trans_with_world_origin_at_center = translate(model_transform, vec3(width / 2, 0, height / 2));
      //  transform.setFromOpenGLMatrix(&trans_with_world_origin_at_center[0][0]);
    }
    bool isDynamic = (mass != 0.f);
    btVector3 localInertia(0, 0, 0);
    if (isDynamic) {
      shape->calculateLocalInertia(mass,
                                   localInertia);
    }
    btDefaultMotionState* motionstate = new btDefaultMotionState(transform);
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(
      mass,  // mass, in kg. 0 -> Static object, will never move.
      motionstate,
      shape,  // collision shape of body
      localInertia    // local inertia
    );
    btRigidBody* rigidBody = new btRigidBody(rigidBodyCI);

    dynamicsWorld->addRigidBody(rigidBody,
      SausageCollisionMasks::ALL,
      SausageCollisionMasks::ALL);
    rigidBodies.push_back(rigidBody);
  }

  void RemoveRigidBody(btRigidBody* body) {
    if (body && body->getMotionState())
    {
      delete body->getMotionState();
    }
    dynamicsWorld->removeCollisionObject(body);
    delete body;
  }
  void Reset() {
    for (auto& body : rigidBodies) {
      delete body->getMotionState();
      dynamicsWorld->removeCollisionObject(body);
      delete body;
    }
    for (auto& shape : collisionShapes)
    {
      delete shape;
    }
    collisionShapes.clear();
    rigidBodies.clear();
  }
  ~PhysicsManager() {
    Reset();
    delete dynamicsWorld;
    delete solver;
    //delete broadphase
    delete overlappingPairCache;
    delete dispatcher;
    delete collisionConfiguration;
  };
};
