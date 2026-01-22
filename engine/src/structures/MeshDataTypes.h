#pragma once

#include "sausage.h"

using namespace std;
using namespace glm;

class BlendTextures;
class MeshData;
class MeshDataStatic;
class MeshDataTerrain;
class MeshDataUI;
class MeshDataOverlay3D;
class MeshDataOutline;
class Vertex;
class VertexStatic;
class VertexUI;
class VertexOutline;

class UniformDataMesh;
class UniformDataMeshStatic;
class UniformDataMeshTerrain;
class UniformDataUI;
class UniformDataOverlay3D;
// placeholder when particular template type not needed
class NotUsedType {};

template<typename TEXTURE_ARRAY_TYPE, typename MESH_TYPE, typename TRANSFORM_TYPE, typename VERTEX_TYPE, typename UNIFORM_DATA_TYPE>
class BufferConsumer;

template<typename TEXTURE_ARRAY_TYPE, typename MESH_TYPE, typename VERTEX_TYPE, typename UNIFORM_DATA_TYPE>
class MeshDataBufferConsumerShared;

using MeshDataBufferConsumerSharedT = MeshDataBufferConsumerShared<BlendTextures, MeshData, Vertex, UniformDataMesh>;
using MeshStaticBufferConsumerSharedT = MeshDataBufferConsumerShared<BlendTextures, MeshDataStatic, VertexStatic, UniformDataMeshStatic>;
using MeshTerrainBufferConsumerSharedT = MeshDataBufferConsumerShared<BlendTextures, MeshDataTerrain, VertexStatic, UniformDataMeshTerrain>;


using MeshDataBufferConsumerT = BufferConsumer<BlendTextures, MeshData, mat4, Vertex, UniformDataMesh>;
using MeshStaticBufferConsumerT = BufferConsumer<BlendTextures, MeshDataStatic, mat4, VertexStatic, UniformDataMeshStatic>;
using MeshTerrainBufferConsumerT = BufferConsumer<BlendTextures, MeshDataTerrain, mat4, VertexStatic, UniformDataMeshTerrain>;

using MeshDataUIConsumerT = BufferConsumer<unsigned int, MeshDataUI, vec2, VertexUI, UniformDataUI>;
using MeshDataOverlay3DConsumerT = BufferConsumer<unsigned int, MeshDataOverlay3D, mat4, VertexStatic, UniformDataOverlay3D>;
using MeshDataOutlineBufferConsumerT = BufferConsumer<unsigned int, MeshDataOutline, vec2, VertexOutline, NotUsedType>;


template <typename MESH_TYPE, typename UNIFORM_DATA_TYPE>
class MeshDataInstance;

using MeshDataInstanceT = MeshDataInstance<MeshData, UniformDataMesh>;
using MeshDataInstanceStaticT = MeshDataInstance<MeshDataStatic, UniformDataMeshStatic>;
using MeshDataInstanceTerrainT = MeshDataInstance<MeshDataTerrain, UniformDataMeshTerrain>;
