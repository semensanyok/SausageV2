#pragma once

#include "sausage.h"
#include "MeshDataStruct.h"
#include "GPUUniformsStruct.h"
#include "InstanceSlot.h"

using namespace std;
using namespace glm;

/**
 * mesh with VertexStatic format, which doesnt contain armature bones (not animated).
 * cite from bullet documentation for btHeightfieldTerrainShape:
 *
 *    The caller is responsible for maintaining the heightfield array; this
 *    class does not make a copy.
 *
 *    The heightfield can be dynamic so long as the min/max height values
 *    capture the extremes (heights must always be in that range).
 *
 *    The local origin of the heightfield is assumed to be the exact
 *    center (as determined by width and length and height, with each
 *    axis multiplied by the localScaling).
 *
 *    \b NOTE: be careful with coordinates.  If you have a heightfield with a local
 *    min height of -100m, and a max height of +500m, you may be tempted to place it
 *    at the origin (0,0) and expect the heights in world coordinates to be
 *    -100 to +500 meters.
 *    Actually, the heights will be -300 to +300m, because bullet will re-center
 *    the heightfield based on its AABB (which is determined by the min/max
 *    heights).  So keep in mind that once you create a btHeightfieldTerrainShape
 *    object, the heights will be adjusted relative to the center of the AABB.  This
 *    is different to the behavior of many rendering engines, but is useful for
 *    physics engines.
 *
 * atm same as MeshData. polymorphism through MeshDataInstance
*/
class MeshDataTerrain : public MeshDataBase, public SausageUserPointer
{
  friend class MeshManager;
public:
  int uniform_offset = -1;
private:
  MeshDataTerrain(unsigned long id, string name = "") :
    MeshDataBase(id, name) {};
  ~MeshDataTerrain() {};
};
