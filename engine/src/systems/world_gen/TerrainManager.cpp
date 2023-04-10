#include "TerrainManager.h"

void TerrainManager::CreateTerrain(int size_x, int size_y, vec3 origin_coord)
{
  vector<vec3> vertices;
  vector<unsigned int> indices;
  vector<vec2> uvs;
  vector<unsigned int> uniform_id;

  // SCALE noise values for mesh and bullet physics.
  int noise_scale = 10;
  auto chunk = CreateChunk(origin_coord, 0, 0, size_x, size_y,
    // OUT
    vertices, indices, uvs, uniform_id, noise_scale);

  vector<vec3> normals = GenNormals(vertices);

  buffer->AllocateStorage(chunk->mesh->slots, vertices.size(), indices.size());
  draw_call_manager->AddNewCommandToDrawCall<MeshDataStatic>(chunk->mesh, draw_call_manager->terrain_dc, 1);
  buffer->BufferMeshData(chunk->mesh, vertices, indices, uvs, normals, uniform_id);

  string name = "Terrain";

  physics_manager->AddTerrainRigidBody(chunk->heightmap, chunk->sizeX, chunk->sizeY, chunk->spacing,
    new MeshDataClickable(name), chunk->mesh->transform, name, -noise_scale, noise_scale);
}

TerrainChunk* TerrainManager::CreateChunk(vec3 pos, int noise_offset_x, int noise_offset_y, int size_x, int size_y,
  // OUT
  vector<vec3>& vertices,
  // OUT
  vector<unsigned int>& indices,
  // OUT
  vector<vec2>& uvs,
  // OUT
  vector<unsigned int>& uniform_id,
  float noise_scale)
{
  TerrainChunk* chunk = new TerrainChunk(size_x, size_y, 1, pos);
  chunk->mesh = mesh_manager->CreateMeshData<MeshDataStatic>();
  chunk->mesh->transform = translate(mat4(1), pos);
  fnSimplex->GenUniformGrid2D(chunk->heightmap.data(), noise_offset_x, noise_offset_y, size_x, size_y, 0.02f, 1337);
  transform(chunk->heightmap.begin(), chunk->heightmap.end(), chunk->heightmap.begin(),
    [&noise_scale](float i) {return i * noise_scale;});
  // create chunk in local space, use transform matrix to apply offsetX/Y
  // each tile has 4 vertices

  // 1. fill simple array and adjastent tiles references
  // distance between vertices
  auto& height_values = chunk->heightmap;
  // set vertices array
  for (int y = 0; y < size_y; y += 1) {
    for (int x = 0; x < size_x; x += 1) {
      // ORIGIN AT SOUTH WEST
      //int x0 = x * chunk->spacing;
      //int y0 = y * chunk->spacing;
      // ORIGIN AT CENTER (for bullet heightmap physics)
      int x0 = x * chunk->spacing - (size_x / 2);
      int y0 = y * chunk->spacing - (size_y / 2);
      int current_row_shift = y * size_x;
      int ind = x + current_row_shift;
      vertices.push_back({ x0, height_values[ind], y0 });
    }
  }
  // TODO: setup 4 vert's chunks
  for (int y = 0; y < size_y; y += 1) {
    for (int x = 0; x < size_x; x += 2) {
      uniform_id.push_back(0);
      uniform_id.push_back(0);
      //uniform_id.push_back(0);
      //uniform_id.push_back(0);

      if (x % 2 == 0) {
        uvs.push_back({ 0.0, 0.0 });
        uvs.push_back({ 0.0, 1.0 });
      }
      else {
        uvs.push_back({ 1.0, 0.0 });
        uvs.push_back({ 1.0, 1.0 });
      }
    }
  }

  // set indices / uv's arrays
  // counter clockwise
  for (int y = 0; y < size_y; y ++) {
    for (int x = 0; x < size_x; x ++) {
      if (x == size_x - 1 || y == size_y - 1) {
        continue;
      }

      int current_row_shift = y * size_x;
      int next_row_shift = (y + 1) * size_x;

      int ind = x + current_row_shift;
      int ind_e = (x + 1) + current_row_shift;
      int ind_n = x + next_row_shift;
      int ind_ne = (x + 1) + next_row_shift;

      indices.push_back(ind);
      indices.push_back(ind_e);
      indices.push_back(ind_n);

      indices.push_back(ind_e);
      indices.push_back(ind_ne);
      indices.push_back(ind_n);
    }
  }
  return chunk;
}

vector<vec3> TerrainManager::GenNormals(vector<vec3> vertices) {
  vector<vec3> normals(vertices.size());
  for (int i = 0; i < vertices.size(); i += 4) {
    vec3 face_normal = cross(vertices[0] - vertices[1], vertices[2] - vertices[3]);
    for (int j = 0; j < 4; j++) {
      normals[i + j] = face_normal;
    }
  }
  assert(normals.size() == vertices.size());
  return normals;
}

void TerrainManager::ReleaseBuffer(TerrainChunk* chunk) {
  buffer->ReleaseSlots(chunk->mesh);
}

void TerrainManager::Deactivate(TerrainChunk* chunk)
{
  draw_call_manager->DisableCommand<MeshDataStatic>(chunk->mesh);
}
