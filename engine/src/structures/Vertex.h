#pragma once

#include "sausage.h"
#include "Interfaces.h"
using namespace glm;

//struct Vertex {
//  virtual int Size() = 0;
//};

/**
 * make sure to group VertexType in contigious series of draws
 */
enum class VertexType {
  NONE = 0,
  // Mesh. Need bones data for animation.
  MESH = 1,
  // Terrain.
  STATIC = 1 << 1,
  UI = 1 << 2,
  OUTLINE = 1 << 3
};

// Animated vertices
struct Vertex {
  vec3 Position;
  vec3 Normal;
  vec2 TexCoords;
  vec3 Tangent;
  vec3 Bitangent;
  ivec4 BoneIds;
  vec4 BoneWeights;
};
// Static vertices
struct VertexStatic {
  vec3 Position;
  vec3 Normal;
  vec2 TexCoords;
  vec3 Tangent;
  vec3 Bitangent;
  // to fetch uniform data, varying per vertex (not per instance id)
  // i.e. terrain chunk split by quads, and each quad assigned its set of blend textures.
  //
  // TODO: Or even transforms, to allow dynamic deform?
  //       To be coherent with physics world, we cannot change vertex positions
  //         so each tile must be assigned its transform matrix.
  //       Or just height variation to save memory,
  //         i.e. `float tile_height_transforms[] = {0}; out.pos += tile_height_transforms[uniform_id]`
  uint UniformId;
};
// vertices for UI elements
struct VertexUI {
  vec3 Position;
  vec3 Normal;
  vec2 TexCoords;
};

// for points/lines draw, for physics debug drawer
struct VertexOutline {
  vec3 Position;
  vec3 Normal; // used as Color; kept name Normal for template polymorphysm in MeshManager.CreateLoadData
};
