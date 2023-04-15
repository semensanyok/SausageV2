#include "TerrainManager.h"

void TerrainManager::CreateTerrain(int size_x, int size_y, vec3 origin_coord)
{
  vector<vec3> vertices;
  vector<unsigned int> indices;
  vector<vec2> uvs;
  vector<unsigned int> uniform_id;

  // SCALE noise values for mesh and bullet physics.
  int noise_scale = 10;
  int noise_offset_x = 0;
  int noise_offset_y = 0;
  float frequency = 0.02f;
  int seed = 1337;
  auto chunk = CreateChunk(origin_coord, noise_offset_x, noise_offset_y, size_x, size_y,
    // OUT
    vertices, indices,
    noise_scale, frequency, seed);

  vector<vec3> normals = GenNormals(vertices);

  buffer->AllocateStorage(chunk->mesh->slots, vertices.size(), indices.size());
  draw_call_manager->AddNewCommandToDrawCall<MeshDataTerrain>(chunk->mesh, draw_call_manager->terrain_dc, 1);


  Texture* tex;
  {
    vector<unsigned int> tex_pixels(size_x * size_y);
    transform(chunk->heightmap.begin(), chunk->heightmap.end(), tex_pixels.begin(),
      [](float i) {return (int)(i * 100000); });
    auto tex_name = (ostringstream() << 
        size_x << size_y << noise_scale << noise_offset_x << noise_offset_y << frequency << seed).str();
    tex = texture_manager->GenTextureArray(tex_pixels.data(), tex_pixels.data(), tex_pixels.data(),
      size_x, size_y, 24, 0, 0x0000ff, 0x00ff00, 0xff0000, 0, 1, tex_name, tex_name, tex_name);
    tex->MakeResident();
  }

  uniform_id.resize(size_x * size_y);
  uvs.resize(size_x * size_y);
  // setup 4 vert's chunks texture data
  for (size_t y = 0; y < size_y; y += 2)
  {
    for (size_t x = 0; x < size_x; x += 2)
    {
      auto tex_indices = chunk->GetTileIndices(x, y);
      // for test - texture with id == 0 and 1.0 weight.
      TerrainBlendTextures* ter_tex;
      BlendTextures blend_tex = { {1.0,tex->id}, 1 };
      if (shared_tiles_textures.find(blend_tex) == shared_tiles_textures.end()) {
        ter_tex = buffer->GetTexturesSlot();
        ter_tex->textures = blend_tex;
        shared_tiles_textures[blend_tex] = ter_tex;
        buffer->BufferTexture(ter_tex, blend_tex);
      }
      else {
        ter_tex = shared_tiles_textures[blend_tex];
      }
      uniform_id[tex_indices.ne] = ter_tex->uniform_id;
      uniform_id[tex_indices.nw] = ter_tex->uniform_id;
      uniform_id[tex_indices.se] = ter_tex->uniform_id;
      uniform_id[tex_indices.sw] = ter_tex->uniform_id;

      uvs[tex_indices.ne] = { 0.0, 0.0 };
      uvs[tex_indices.nw] = { 0.0, 1.0 };
      uvs[tex_indices.se] = { 1.0, 0.0 };
      uvs[tex_indices.sw] = { 1.0, 1.0 };
    }
  }
  buffer->BufferMeshData(chunk->mesh, vertices, indices, uvs, normals, uniform_id);

  string name = "Terrain";

  physics_manager->AddTerrainRigidBody(chunk->heightmap, chunk->size_x, chunk->size_y, chunk->spacing,
    new MeshDataClickable(name), chunk->mesh->transform, name, -noise_scale, noise_scale);
}

TerrainChunk* TerrainManager::CreateChunk(vec3 pos, int noise_offset_x, int noise_offset_y, int size_x, int size_y,
  // OUT
  vector<vec3>& vertices,
  // OUT
  vector<unsigned int>& indices,
  float noise_scale,
  float frequency,
  int seed)
{
  TerrainChunk* chunk = new TerrainChunk(size_x, size_y, 1, pos);
  chunk->mesh = mesh_manager->CreateMeshData<MeshDataTerrain>();
  chunk->mesh->transform = translate(mat4(1), pos);
  fnSimplex->GenUniformGrid2D(chunk->heightmap.data(), noise_offset_x, noise_offset_y, size_x, size_y, frequency, seed);
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
  // TODO: x+=2 ????
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
