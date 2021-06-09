#pragma once

#include "sausage.h"
#include "btBulletDynamicsCommon.h"

class PhysicsManager {
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btBroadphaseInterface* overlappingPairCache;
	
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;
public:
	PhysicsManager() {
		collisionConfiguration = new btDefaultCollisionConfiguration();
		dispatcher = new btCollisionDispatcher(collisionConfiguration);
		overlappingPairCache = new btDbvtBroadphase();
		solver = new btSequentialImpulseConstraintSolver;
		
		dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
		dynamicsWorld->setGravity(btVector3(0, -9.8, 0));
		
		btBoxShape* colShape = new btBoxShape(btVector3(plankWidth, plankHeight, plankBreadth));
		
		btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(
			0,                  // mass, in kg. 0 -> Static object, will never move.
			motionstate,
			boxCollisionShape,  // collision shape of body
			btVector3(0, 0, 0)    // local inertia
		);
		btRigidBody* rigidBody = new btRigidBody(rigidBodyCI);

		dynamicsWorld->addRigidBody(rigidBody);

	}
	~PhysicsManager() {};
}