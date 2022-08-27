#pragma once

#include "sausage.h"
#include "structures/Structures.h"
#include "structures/Interfaces.h"
#include "structures/PhysicsStruct.h"
#include "buffer/BufferStorage.h"
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
	
	PhysicsManager(StateManager* state_manager) : state_manager{ state_manager } {
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
		//dynamicsWorld->stepSimulation(GameSettings::delta_time, 1);
    IF_PROFILE_ENABLED(auto proft3 = chrono::steady_clock::now(););
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
		if (GameSettings::phys_debug_draw) {
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
			SausageUserPointer* sausage_up = ((SausageUserPointer*)rigidBody->getUserPointer());
      MeshData* mesh_data = nullptr;
      if (dynamic_cast<MeshDataClickable*>(sausage_up)) {
        mesh_data = ((MeshDataClickable*)sausage_up)->mesh_data;
      }
      else if (dynamic_cast<MeshDataBase*>(sausage_up)) {
          mesh_data = ((MeshData*)sausage_up);
      }
      auto& update = state_manager->GetPhysicsUpdate(mesh_data);
			update.first = mesh_data;
			btTrans.getOpenGLMatrix(&update.second[0][0]);
		}
    IF_PROFILE_ENABLED(ProfTime::physics_buf_trans_ns = chrono::steady_clock::now() - proft4);
	}
	void AddBoxRigidBody(PhysicsData* physics_data,
    SausageUserPointer* user_pointer,
    mat4& model_transform,
    string& name_for_log) {
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
