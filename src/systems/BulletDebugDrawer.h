#pragma once

#include "sausage.h"

class BulletDebugDrawer : public btIDebugDraw
{
    int m_debugMode;

public:

    BulletDebugDrawer(Renderer* renderer) {};

    virtual void   drawLine(const btVector3& from, const btVector3& to, const btVector3& color);

    virtual void   drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);

    virtual void   reportErrorWarning(const char* warningString);

    virtual void   draw3dText(const btVector3& location, const char* textString);

    virtual void   setDebugMode(int debugMode);

    virtual int      getDebugMode() const { return m_debugMode; }

};
void   BulletDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
};
void   BulletDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {
};
void   BulletDebugDrawer::reportErrorWarning(const char* warningString) {
};
void   BulletDebugDrawer::draw3dText(const btVector3& location, const char* textString) {
};
void   BulletDebugDrawer::setDebugMode(int debugMode) {
};
