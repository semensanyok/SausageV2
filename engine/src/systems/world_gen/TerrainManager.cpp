#include "TerrainManager.h"

void TerrainManager::CreateTerrain(int size_x, int size_y)
{
  vector<vec3> vertices;
  vertices.reserve(size_x * size_y);
  vector<unsigned int> indices;
  vector<vec2> uvs(size_x * size_y);

  auto chunk = CreateChunk(vec3(0, 0, 0), 0, 0, 100, 100,
    // OUT
    vertices, indices, uvs);

  vector<vec3> normals = GenNormals(vertices);
  
  buffer->BufferMeshData(chunk->mesh, vertices, indices, uvs, normals);

  draw_call_manager->AddNewCommandToDrawCall<MeshDataStatic>(chunk->mesh, draw_call_manager->mesh_static_dc, 1);
}

TerrainChunk* TerrainManager::CreateChunk(vec3 pos, int noise_offset_x, int noise_offset_y, int size_x, int size_y,
  // OUT
  vector<vec3>& vertices,
  // OUT
  vector<unsigned int>& indices,
  // OUT
  vector<vec2>& uvs)
{
  const int SIZE = 1;
  TerrainChunk* chunk = new TerrainChunk(size_x, size_y, SIZE, pos);
  chunk->mesh = mesh_manager->CreateMeshData<MeshDataStatic>();
  fnSimplex->GenUniformGrid2D(chunk->heightmap.data(), noise_offset_x, noise_offset_y, size_x, size_y, 0.02f, 1337);

  // create chunk in local space, use transform matrix to apply offsetX/Y
  // each tile has 4 vertices

  // 1. fill simple array and adjastent tiles references
  auto& height_values = chunk->heightmap;
  for (int y = 0; y < size_y; y+=2) {
    for (int x = 0; x < size_x; x += 2) {
      int x0 = x;
      int x1 = x + 1;
      int y0 = y;
      int y1 = y + 1;

      int current_row_shift = y * size_x;
      int next_row_shift = y1 * size_x;

      int ind = x0 + current_row_shift;
      int ind_e = x1 + current_row_shift;
      int ind_ne = x1 + next_row_shift;
      int ind_n = x0 + next_row_shift;

      size_t start = vertices.size();
      vertices.push_back({ x0, y0, height_values[ind] });
      vertices.push_back({ x0, y1, height_values[ind_n] });
      vertices.push_back({ x1, y0, height_values[ind_e] });
      vertices.push_back({ x1, y1, height_values[ind_ne] });

      uvs[start] = { 0.0, 0.0 };
      uvs[start + 1] = { 0.0, 1.0 };
      uvs[start + 2] = { 1.0, 0.0 };
      uvs[start + 3] = { 1.0, 1.0 };

      // 1 triangle
      indices.push_back(start);
      indices.push_back(start + 1);
      indices.push_back(start + 2);

      //indices.push_back(ind1);
      //indices.push_back(ind1_next_row);
      //indices.push_back(ind2);

      // 2 triangle
      indices.push_back(start + 2);
      indices.push_back(start + 3);
      indices.push_back(start + 1);

      //indices.push_back(ind2);
      //indices.push_back(ind2_next_row);
      //indices.push_back(ind1_next_row);
     }
  }
  return chunk;
}

vector<vec3> TerrainManager::GenNormals(vector<vec3> vertices) {
  vector<vec3> normals(vertices.size());
  // need to calculate vertex normal as mean of 4 faces normals
  // but we have instanced draw call, same normal for all tiles...
  // but transform mat4 should adjust each vertex normal to be equal to face normal
  // TODO: check that:
  //  expected to see seems between tiles and not smooth shading, each tile having constant light, no interpolation
  //  normal texture should make it a bit less visible though
  //out_face_normal = cross(vertices[0] - vertices[1], vertices[2] - vertices[3]);
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
//void TerrainManager::CreateTerrain() {
  //SystemsManager* systems_manager = SystemsManager::GetInstance();
  //MeshManager* mesh_manager = systems_manager->mesh_manager;
  //BufferManager* buffer_manager = systems_manager->buffer_manager;
  //TextureManager* texture_manager = systems_manager->texture_manager;
  //ShaderManager* shader_manager = systems_manager->shader_manager;
  //MeshDataBufferConsumer* buffer = buffer_manager->buffer;

  //const int SCALE = 1;
  //const int sizeX = 100;
  //const int sizeY = 100;

  //vector<vec3> vertices(sizeX * sizeY);
  //vector<unsigned int> indices;
  //vector<float> noiseOutput(sizeX * sizeY);
  //vector<vec3> normals;
  //vector<vec2> uvs(sizeX * sizeY);

  //fnSimplex->GenUniformGrid2D(noiseOutput.data(), 0, 0, sizeX, sizeY, 0.02f, 1337);
  //for (int y = 0; y < sizeY; y++) {
  //  for (int x = 0; x < sizeX; x += 1) {
  //    if (x == sizeX - 1) {
  //      break;
  //    }
  //    int x1 = x;
  //    int x2 = x + 1;
  //    int current_row_shift = y * sizeX;
  //    int ind1 = x1 + current_row_shift;
  //    int ind2 = x2 + current_row_shift;

  //    vec3& vert1 = vertices[ind1];
  //    vert1.x = SCALE * x;
  //    vert1.y = SCALE * noiseOutput[ind1];
  //    vert1.z = SCALE * y;

  //    vec3& vert2 = vertices[ind2];
  //    vert2.x = SCALE * x2;
  //    vert2.y = SCALE * noiseOutput[ind2];
  //    vert2.z = SCALE * y;


  //    // 2 vertices in current row + 2 vertices next row
  //    // to form rectangle
  //    // and assign repeated texture UV's
  //    // skip last row
  //    if (y < sizeY - 1) {
  //      int next_row_shift = (y + 1) * sizeX;
  //      int ind1_next_row = x1 + next_row_shift;
  //      int ind2_next_row = x2 + next_row_shift;

  //      // 1 triangle
  //      indices.push_back(ind1);
  //      indices.push_back(ind1_next_row);
  //      indices.push_back(ind2);
  //      // 2 triangle
  //      indices.push_back(ind2);
  //      indices.push_back(ind2_next_row);
  //      indices.push_back(ind1_next_row);

  //      if (x % 2 == 0 && y % 2 == 0) {
  //        uvs[ind1] = { 0.0, 0.0 };
  //        uvs[ind2] = { 0.0, 1.0 };
  //        uvs[ind1_next_row] = { 1.0, 0.0 };
  //        uvs[ind2_next_row] = { 1.0, 1.0 };
  //      }
  //    }
  //  }
  //}

  /*
  * TODO:
  *   generate normals, from texture or slope (diff between height values?)
  */

  // ----- 1. BUFFER MESH DATA AND PREPARE DRAW CALL COMMON MACHINERY   --------------------
  // add drawcall
  //auto draw_call = buffer->CreateDrawCall(
  //  shader_manager->all_shaders->blinn_phong,
  //  buffer->CreateCommandBuffer(BufferSettings::MAX_COMMAND),
  //  GL_TRIANGLES);
  //draw_call->buffer->ActivateCommandBuffer(draw_call->command_ptr);
  //systems_manager->renderer->AddDraw(draw_call, DrawOrder::MESH);

  //// load data
  //vector<shared_ptr<MeshLoadData>> load_data = { mesh_manager->CreateMesh(vertices, indices, normals, uvs) };
  //load_data[0]->tex_names = new MaterialTexNames("checker1.png", EMPTY_STRING, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING);
  //vector<MeshDataBase*> meshes = { mesh_manager->CreateMeshData(load_data[0].get()) };
  //auto mesh = (MeshData*)meshes[0];

  //// buffer data
  //buffer->
  //  BufferMeshData(meshes, load_data);
  //buffer->
  //  SetBaseMeshForInstancedCommand(meshes, load_data);
  //buffer->BufferTransform(mesh);

  //// load texture to buffer
  //Texture* texture = texture_manager->LoadTextureArray(load_data[0]->tex_names);
  //texture->MakeResident();

  //mesh->textures = { {1.0, texture->id}, 1 };
  //buffer->BufferMeshTexture(mesh);

  //float light_power = 1.0;
  //// add light for blinn_phong to view texture
  //vector<Light*> light1 = { new Light{ {0,-1,0,0},
  //                     { 0.0, 5.4625, 0.0,0},
  //                     {light_power ,light_power ,light_power ,0},
  //                     {light_power ,light_power ,light_power ,0},
  //                     LightType::Directional,
  //                     1.00000000,
  //                     0.699999988,
  //                     0.699999988,
  //                     10,
  //                     10 } };
  //buffer->BufferLights(light1);
  //// add command to drawcall
  //vector<DrawElementsIndirectCommand> commands = { mesh->command };
  //draw_call->command_count = commands.size();
  //draw_call->buffer->AddCommands(commands, draw_call->command_ptr);
  //CheckGLError();
  // -------- 1. END ---------------------------------------------------------------------
//}
