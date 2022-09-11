#include "TerrainManager.h"

TerrainChunk* TerrainManager::CreateChunk(int size_x, int size_y, int noise_offset_x, int noise_offset_y)
{
  vector<vec3> vertices(size_x * size_y);
  vector<unsigned int> indices;

  vector<float> height_values(size_x * size_y);

  // TODO_1 (AHORA): calculate each vertex normal as mean of adjastent faces normals
  //1 2 3
  //  4 5 6
  //  7 8 9
  //  normal of vertex 5 is mean of normals of faces
  //  [(1 2 4 5), (2 3 5 6), (4, 5, 7, 8), (5, 6, 8, 9)]
  //normal of face 1 2 4 5 == (1 - 2) x(1 - 4), where 1 - 4 - vertices(x, y, z)
  //  TBN IS NEEDED IN SHADER !!!
  // vector<vec2> normals;

  vector<float> moisture_values(size_x * size_y);
  vector<float> temperature_values(size_x * size_y);

  vector<vec3> normals;
  vector<vec2> uvs(size_x * size_y);

  fnSimplex->GenUniformGrid2D(height_values.data(), noise_offset_x, noise_offset_y, size_x, size_y, 0.02f, 1337);
  fnSimplex->GenUniformGrid2D(moisture_values.data(), noise_offset_x, noise_offset_y, size_x, size_y, 0.02f, 1337);
  fnSimplex->GenUniformGrid2D(temperature_values.data(), noise_offset_x, noise_offset_y, size_x, size_y, 0.02f, 1337);

  TerrainChunk* chunk = new TerrainChunk(size_x, size_y);

  // create chunk in local space, use transform matrix to apply offsetX/Y
  // each tile has 4 vertices

  // 1. fill simple array and adjastent tiles references
  for (int y = 0; y < size_y; y++) {
    for (int x = 0; x < size_x; x++) {
      int x1 = x;
      int x2 = x + 1;
      int y1 = y;
      int y2 = y + 1;
      int current_row_shift = y * size_x;
      int next_row_shift = y2 * size_x;

      int ind = x1 + current_row_shift;
      int ind_e = x2 + current_row_shift;
      int ind_se = x2 + next_row_shift;
      int ind_s = x1 + next_row_shift;

      TerrainTile* current_tile = chunk->tiles[ind];
      if (x == 0) {
        int prev_row_shift = (y - 1) * size_x;

        int ind_w = x1 - 1 + current_row_shift;
        int ind_nw = (x1 - 1) + prev_row_shift;
        int ind_n = x1 + prev_row_shift;
        int ind_ne = (x1 + 1) + prev_row_shift;
        int ind_sw = (x1 - 1) + next_row_shift;

        AdjastentTiles& adj = current_tile->adjastent;

        bool is_east_border = x2 >= size_x;
        bool is_south_border = y2 >= size_y;
        bool is_west_border = x1 == 0;
        bool is_north_border = y1 == 0;

        adj.e = is_east_border ? nullptr : chunk->tiles[ind_e];
        adj.se = is_east_border || is_south_border ? nullptr : chunk->tiles[ind_se];
        adj.s = is_south_border ? nullptr : chunk->tiles[ind_s];
        adj.w = is_west_border ? nullptr : chunk->tiles[ind_w];
        adj.nw = is_west_border || is_north_border ? nullptr : chunk->tiles[ind_nw];
        adj.n = is_north_border ? nullptr : chunk->tiles[ind_n];
        adj.ne = is_north_border || is_east_border ? nullptr : chunk->tiles[ind_ne];
        adj.sw = is_south_border || is_west_border ? nullptr : chunk->tiles[ind_sw];
      }

      TerrainPixelValues& pixel_values = current_tile->pixel_values;
      pixel_values.height = (
        height_values[ind]
        + height_values[ind_e]
        + height_values[ind_se]
        + height_values[ind_s]
       ) / 4;
      pixel_values.moisture = (
        moisture_values[ind]
        + moisture_values[ind_e]
        + moisture_values[ind_se]
        + moisture_values[ind_s]
       ) / 4;
      pixel_values.temperature = (
        temperature_values[ind]
        + temperature_values[ind_e]
        + temperature_values[ind_se]
        + temperature_values[ind_s]
       ) / 4;
    }
  }
  return chunk;
}

TerrainChunk* TerrainManager::BufferTerrain(int world_offset_x, int world_offset_y)
{
  return nullptr;
}

void TerrainManager::CreateTerrain() {
  SystemsManager* systems_manager = SystemsManager::GetInstance();
  MeshManager* mesh_manager = systems_manager->mesh_manager;
  BufferManager* buffer_manager = systems_manager->buffer_manager;
  TextureManager* texture_manager = systems_manager->texture_manager;
  ShaderManager* shader_manager = systems_manager->shader_manager;
  MeshDataBufferConsumer* mesh_data_buffer = buffer_manager->mesh_data_buffer;

  int sizeX = 100;
  int sizeY = 100;

  vector<vec3> vertices(sizeX * sizeY);
  vector<unsigned int> indices;
  vector<float> noiseOutput(sizeX * sizeY);
  vector<vec3> normals;
  vector<vec2> uvs(sizeX * sizeY);

  fnSimplex->GenUniformGrid2D(noiseOutput.data(), 0, 0, sizeX, sizeY, 0.02f, 1337);
  for (int y = 0; y < sizeY; y++) {
    for (int x = 0; x < sizeX; x += 1) {
      if (x == sizeX - 1) {
        break;
      }
      int x1 = x;
      int x2 = x + 1;
      int current_row_shift = y * sizeX;
      int ind1 = x1 + current_row_shift;
      int ind2 = x2 + current_row_shift;

      vec3& vert1 = vertices[ind1];
      vert1.x = SCALE * x;
      vert1.y = SCALE * noiseOutput[ind1];
      vert1.z = SCALE * y;

      vec3& vert2 = vertices[ind2];
      vert2.x = SCALE * x2;
      vert2.y = SCALE * noiseOutput[ind2];
      vert2.z = SCALE * y;


      // 2 vertices in current row + 2 vertices next row
      // to form rectangle
      // and assign repeated texture UV's
      // skip last row
      if (y < sizeY - 1) {
        int next_row_shift = (y + 1) * sizeX;
        int ind1_next_row = x1 + next_row_shift;
        int ind2_next_row = x2 + next_row_shift;

        // 1 triangle
        indices.push_back(ind1);
        indices.push_back(ind1_next_row);
        indices.push_back(ind2);
        // 2 triangle
        indices.push_back(ind2);
        indices.push_back(ind2_next_row);
        indices.push_back(ind1_next_row);

        if (x % 2 == 0 && y % 2 == 0) {
          uvs[ind1] = { 0.0, 0.0 };
          uvs[ind2] = { 0.0, 1.0 };
          uvs[ind1_next_row] = { 1.0, 0.0 };
          uvs[ind2_next_row] = { 1.0, 1.0 };
        }
      }
    }
  }

  /*
  * TODO:
  *   generate normals, from texture or slope (diff between height values?)
  */

  // ----- 1. BUFFER MESH DATA AND PREPARE DRAW CALL COMMON MACHINERY   --------------------
  // add drawcall
  auto draw_call = mesh_data_buffer->CreateDrawCall(
    shader_manager->all_shaders->blinn_phong,
    mesh_data_buffer->CreateCommandBuffer(BufferSettings::MAX_COMMAND),
    GL_TRIANGLES);
  draw_call->buffer->ActivateCommandBuffer(draw_call->command_buffer);
  systems_manager->renderer->AddDraw(draw_call, DrawOrder::MESH);

  // load data
  vector<shared_ptr<MeshLoadData>> load_data = { mesh_manager->CreateMesh(vertices, indices, normals, uvs) };
  load_data[0]->tex_names = new MaterialTexNames("checker1.png", EMPTY_STRING, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING);
  vector<MeshDataBase*> meshes = { mesh_manager->CreateMeshData(load_data[0].get()) };
  auto mesh = (MeshData*)meshes[0];

  // buffer data
  mesh_data_buffer->
    BufferMeshData(meshes, load_data);
  mesh_data_buffer->
    SetBaseMeshForInstancedCommand(meshes, load_data);
  mesh_data_buffer->BufferTransform(mesh);

  // load texture to buffer
  Texture* texture = texture_manager->LoadTextureArray(load_data[0]->tex_names);
  texture->MakeResident();

  mesh->textures = { {1.0, texture->id}, 1 };
  mesh_data_buffer->BufferMeshTexture(mesh);

  float light_power = 1.0;
  // add light for blinn_phong to view texture
  vector<Light*> light1 = { new Light{ {0,-1,0,0},
                       { 0.0, 5.4625, 0.0,0},
                       {light_power ,light_power ,light_power ,0},
                       {light_power ,light_power ,light_power ,0},
                       LightType::Directional,
                       1.00000000,
                       0.699999988,
                       0.699999988,
                       10,
                       10 } };
  mesh_data_buffer->BufferLights(light1);
  // add command to drawcall
  vector<DrawElementsIndirectCommand> commands = { mesh->command };
  draw_call->command_count = commands.size();
  draw_call->buffer->AddCommands(commands, draw_call->command_buffer);
  CheckGLError();
  // -------- 1. END ---------------------------------------------------------------------
}
