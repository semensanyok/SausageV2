#pragma once

#include "sausage.h"
#include "Structures.h"
#include "BufferStorage.h"
#include "BulletDebugDrawer.h"
#include "Settings.h"
#include "../utils/AssetUtils.h"
#include "StateManager.h"

using namespace std;

class PhysicsManager {
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btBroadphaseInterface* overlappingPairCache;
	BulletDebugDrawer* debugDrawer;

	vector<btRigidBody*> rigidBodies;
	// TODO: reuse colision shapes. (from bullet docs)
	vector<btCollisionShape*> collisionShapes;

	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;

	const char* save_name = "./testFile.bullet";
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
#ifdef SAUSAGE_PROFILE_ENABLE
		auto proft3 = chrono::steady_clock::now();
#endif
		dynamicsWorld->stepSimulation(state_manager->delta_time * GameSettings::physics_step_multiplier, 1000);
#ifdef SAUSAGE_PROFILE_ENABLE
		ProfTime::physics_sym_ns = chrono::steady_clock::now() - proft3;
#endif
#ifdef SAUSAGE_PROFILE_ENABLE
		auto proft8 = chrono::steady_clock::now();
#endif
#ifdef SAUSAGE_DEBUG_DRAW_PHYSICS
		dynamicsWorld->debugDrawWorld();
#endif
#ifdef SAUSAGE_PROFILE_ENABLE
		ProfTime::physics_debug_draw_world_ns = chrono::steady_clock::now() - proft8;
#endif
	}
	void ClickRayTest(float screen_x, float screen_y, const vec3& position, float distance, const mat4& camera_projection_view_inverse) {
		auto screen_x_normalized = (screen_x / GameSettings::SCR_WIDTH - 0.5f) * 2.0;
		auto screen_y_normalized = -((screen_y) / GameSettings::SCR_HEIGHT - 0.5f) * 2.0;
		vec4 world_end = camera_projection_view_inverse * vec4(screen_x_normalized, screen_y_normalized, 1, 1);
		world_end = world_end * distance;

		btVector3 btStart(position.x, position.y, position.z);
		btVector3 btEnd(world_end.x, world_end.y, world_end.z);

		btCollisionWorld::ClosestRayResultCallback RayCallback(
			btStart,
			btEnd
		);

		dynamicsWorld->rayTest(
			btStart,
			btEnd,
			RayCallback
		);

		string message;
		if (RayCallback.hasHit()) {
			std::ostringstream oss;
			oss << "mesh " << ((MeshData*)RayCallback.m_collisionObject->getUserPointer())->name;
			message = oss.str();
		}
		else {
			message = "background";
		}
#ifdef SAUSAGE_DEBUG_DRAW_PHYSICS
		if (GameSettings::phys_debug_draw) {
			debugDrawer->drawLinePersist(btStart, btEnd, { 255,0,0 });
		}
#endif
		cout << message << endl;
	}
	void UpdateTransforms() {
#ifdef SAUSAGE_PROFILE_ENABLE
		auto proft4 = chrono::steady_clock::now();
#endif
		auto nonStatic = dynamicsWorld->getNonStaticRigidBodies();
		btTransform btTrans;
		for (size_t i = 0; i < nonStatic.size(); i++)
		{
			auto rigidBody = nonStatic[i];
			rigidBody->getMotionState()->getWorldTransform(btTrans);
			MeshData* mesh_data = (MeshData*)rigidBody->getUserPointer();
			auto& update = state_manager->GetPhysicsUpdate(mesh_data);
			update.first = mesh_data;
			btTrans.getOpenGLMatrix(&update.second[0][0]);
		}
#ifdef SAUSAGE_PROFILE_ENABLE
		ProfTime::physics_buf_trans_ns = chrono::steady_clock::now() - proft4;
#endif
	}
	void AddBoxRigidBody(vec3 min_AABB, vec3 max_AABB, float mass, MeshData* user_pointer, mat4& model_transform) {
		vec3 half_extents = max_AABB - min_AABB;
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
				<< (user_pointer == nullptr ? string("Unknown user pointer") : user_pointer->name)).str());
		}
		transform.setFromOpenGLMatrix(&model_transform[0][0]);
		btDefaultMotionState* motionstate = new btDefaultMotionState(transform);

		bool isDynamic = (mass != 0.f);
		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			shape->calculateLocalInertia(mass, localInertia);

		btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(
			mass,                  // mass, in kg. 0 -> Static object, will never move.
			motionstate,
			shape,  // collision shape of body
			localInertia    // local inertia
		);
		btRigidBody* rigidBody = new btRigidBody(rigidBodyCI);
		
		rigidBody->setUserPointer(user_pointer);
		dynamicsWorld->addRigidBody(rigidBody);
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
	void Serialize() {
		btDefaultSerializer* serializer = new btDefaultSerializer();
		dynamicsWorld->serialize(serializer);
		FILE* file = fopen(save_name, "wb");
		fwrite(serializer->getBufferPointer(), serializer->getCurrentBufferSize(), 1, file);
		fclose(file);
	}

	~PhysicsManager() {
		for (auto& body : rigidBodies) {
			delete body->getMotionState();
			dynamicsWorld->removeCollisionObject(body);
			delete body;
		}
		for (auto& shape : collisionShapes)
		{
			delete shape;
		}
		delete dynamicsWorld;
		delete solver;
		//delete broadphase
		delete overlappingPairCache;
		delete dispatcher;
		delete collisionConfiguration;
	};
};