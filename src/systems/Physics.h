#pragma once

#include "sausage.h"
#include "Structures.h"
#include "BufferStorage.h"
#include "BulletDebugDrawer.h"

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
public:
	PhysicsManager(BulletDebugDrawer* debugDrawer = nullptr) : debugDrawer{ debugDrawer } {
		collisionConfiguration = new btDefaultCollisionConfiguration();
		dispatcher = new btCollisionDispatcher(collisionConfiguration);
		overlappingPairCache = new btDbvtBroadphase();
		solver = new btSequentialImpulseConstraintSolver;

		dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
		dynamicsWorld->setGravity(btVector3(0, -9.8, 0));
		if (debugDrawer != nullptr) {
			dynamicsWorld->setDebugDrawer(debugDrawer);
			dynamicsWorld->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawWireframe + btIDebugDraw::DBG_DrawContactPoints);
		}
	}
	void Simulate(float deltatime) {
		dynamicsWorld->stepSimulation(deltatime);
	}
	void UpdateTransforms() {
		auto nonStatic = dynamicsWorld->getNonStaticRigidBodies();
		btTransform btTrans;
		for (size_t i = 0; i < nonStatic.size(); i++)
		{
			auto rigidBody = nonStatic[i];
			rigidBody->getMotionState()->getWorldTransform(btTrans);
			MeshData* mesh_data = (MeshData*)rigidBody->getUserPointer();
			btTrans.getOpenGLMatrix(&(mesh_data->transform[0][0]));
			mesh_data->buffer->BufferTransform(*mesh_data);
		}
	}
	void AddBoxRigidBody(vec3 min_AABB, vec3 max_AABB, float mass, void* user_pointer, mat4& model_transform) {
		vec3 half_extents = abs(max_AABB - min_AABB);
		half_extents.x /= 2;
		half_extents.y /= 2;
		half_extents.z /= 2;
		auto datam = ((MeshData*)user_pointer);

		btBoxShape* shape = new btBoxShape(btVector3(half_extents.x, half_extents.y, half_extents.x));
		collisionShapes.push_back(shape);
		auto transform = btTransform();
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
		//next line is optional: it will be cleared by the destructor when the array goes out of scope
		collisionShapes.clear();
	};
};