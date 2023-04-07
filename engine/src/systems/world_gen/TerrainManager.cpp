#include "TerrainManager.h"

void TerrainManager::CreateTerrain(int size_x, int size_y)
{
  vector<vec3> vertices;
  vector<unsigned int> indices;
  vector<vec2> uvs;

  auto chunk = CreateChunk(vec3(0, 0, 0), 0, 0, size_x, size_y,
    // OUT
    vertices, indices, uvs);

  vector<vec3> normals = GenNormals(vertices);

  buffer->AllocateStorage(chunk->mesh->slots, vertices.size(), indices.size());
  draw_call_manager->AddNewCommandToDrawCall<MeshDataStatic>(chunk->mesh, draw_call_manager->mesh_static_dc, 1);
  buffer->BufferMeshData(chunk->mesh, vertices, indices, uvs, normals);

  string name = "Terrain";
  physics_manager->AddTerrainRigidBody(chunk->heightmap, chunk->sizeX, chunk->sizeY, chunk->spacing,
    new MeshDataClickable(name), chunk->mesh->transform, name);
}

TerrainChunk* TerrainManager::CreateChunk(vec3 pos, int noise_offset_x, int noise_offset_y, int size_x, int size_y,
  // OUT
  vector<vec3>& vertices,
  // OUT
  vector<unsigned int>& indices,
  // OUT
  vector<vec2>& uvs)
{
  TerrainChunk* chunk = new TerrainChunk(size_x, size_y, 1, pos);
  chunk->mesh = mesh_manager->CreateMeshData<MeshDataStatic>();
  fnSimplex->GenUniformGrid2D(chunk->heightmap.data(), noise_offset_x, noise_offset_y, size_x, size_y, 0.02f, 1337);

  // create chunk in local space, use transform matrix to apply offsetX/Y
  // each tile has 4 vertices

  // 1. fill simple array and adjastent tiles references
  // distance between vertices
  auto& height_values = chunk->heightmap;
  // set vertices array
  for (int y = 0; y < size_y; y += 1) {
    for (int x = 0; x < size_x; x += 1) {
      int x0 = x * chunk->spacing;
      int y0 = y * chunk->spacing;
      int current_row_shift = y * size_x;
      int ind = x + current_row_shift;
      vertices.push_back({ x0, height_values[ind], y0 });
    }
  }
  // set indices / uv's arrays
  // counter clockwise
  for (int y = 0; y < size_y; y ++) {
    for (int x = 0; x < size_x; x ++) {
      if (x == size_x - 1 || y == size_y - 1) {
        continue;
      }
      uvs.push_back({ 0.0, 0.0 });
      uvs.push_back({ 0.0, 1.0 });
      uvs.push_back({ 1.0, 0.0 });
      uvs.push_back({ 1.0, 1.0 });

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

// Test create terrain, first prototype
void TerrainManager::CreateTerrain() {
//  const int SCALE = 1;
//  const int sizeX = 100;
//  const int sizeY = 100;
//
//  vector<vec3> vertices(sizeX * sizeY);
//  vector<unsigned int> indices;
//  vector<float> noiseOutput(sizeX * sizeY);
//  vector<vec3> normals;
//  vector<vec2> uvs(sizeX * sizeY);
//
//  fnSimplex->GenUniformGrid2D(noiseOutput.data(), 0, 0, sizeX, sizeY, 0.02f, 1337);
//  for (int y = 0; y < sizeY; y++) {
//    for (int x = 0; x < sizeX; x += 1) {
//      if (x == sizeX - 1) {
//        break;
//      }
//      int x1 = x;
//      int x2 = x + 1;
//      int current_row_shift = y * sizeX;
//      int ind1 = x1 + current_row_shift;
//      int ind2 = x2 + current_row_shift;
//
//      vec3& vert1 = vertices[ind1];
//      vert1.x = SCALE * x;
//      vert1.y = SCALE * noiseOutput[ind1];
//      vert1.z = SCALE * y;
//
//      vec3& vert2 = vertices[ind2];
//      vert2.x = SCALE * x2;
//      vert2.y = SCALE * noiseOutput[ind2];
//      vert2.z = SCALE * y;
//
//
//       //2 vertices in current row + 2 vertices next row
//       //to form rectangle
//       //and assign repeated texture UV's
//       //skip last row
//      if (y < sizeY - 1) {
//        int next_row_shift = (y + 1) * sizeX;
//        int ind1_next_row = x1 + next_row_shift;
//        int ind2_next_row = x2 + next_row_shift;
//
//        // 1 triangle
//      indices.push_back(ind1);
//      indices.push_back(ind1_next_row);
//      indices.push_back(ind2);
//      // 2 triangle
//      indices.push_back(ind2);
//      indices.push_back(ind2_next_row);
//      indices.push_back(ind1_next_row);
//
//      if (x % 2 == 0 && y % 2 == 0) {
//        uvs[ind1] = { 0.0, 0.0 };
//        uvs[ind2] = { 0.0, 1.0 };
//        uvs[ind1_next_row] = { 1.0, 0.0 };
//        uvs[ind2_next_row] = { 1.0, 1.0 };
//      }
//    }
//  }
//}
//
//  /*
//  * TODO:
//  *   generate normals, from texture or slope (diff between height values?)
//  */
//
//  
//  // load data
//  vector<shared_ptr<MeshLoadData<VertexStatic>>> load_data = { mesh_manager->CreateLoadData<VertexStatic>(vertices, indices, normals, uvs) };
//  //load_data[0]->tex_names = new MaterialTexNames("checker1.png", EMPTY_STRING, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING);
//  vector<MeshDataBase*> meshes = { mesh_manager->CreateMeshData<VertexStatic, MeshDataStatic>(load_data[0]) };
//  auto mesh = (MeshData*)meshes[0];
//  
//  // buffer data
//  buffer->
//    BufferMeshData(meshes, load_data);
//  buffer->
//    SetBaseMeshForInstancedCommand(meshes, load_data);
//  buffer->BufferTransform(mesh);
//  
//  // load texture to buffer
//  Texture* texture = texture_manager->LoadTextureArray(load_data[0]->tex_names);
//  texture->MakeResident();
//  
//  mesh->textures = { {1.0, texture->id}, 1 };
//  buffer->BufferMeshTexture(mesh);
//  
//  float light_power = 1.0;
//  // add light for blinn_phong to view texture
//  vector<Light*> light1 = { new Light{ {0,-1,0,0},
//                       { 0.0, 5.4625, 0.0,0},
//                       {light_power ,light_power ,light_power ,0},
//                       {light_power ,light_power ,light_power ,0},
//                       LightType::Directional,
//                       1.00000000,
//                       0.699999988,
//                       0.699999988,
//                       10,
//                       10 } };
//  buffer->BufferLights(light1);
//  // add command to drawcall
//  vector<DrawElementsIndirectCommand> commands = { mesh->command };
//  draw_call->command_count = commands.size();
//  draw_call->buffer->AddCommands(commands, draw_call->command_ptr);
//  CheckGLError();
  //-------- 1. END ---------------------------------------------------------------------
}
