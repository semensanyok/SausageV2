#include "TerrainManager.h"

void TerrainManager::CreateTerrain()
{
  auto chunk = CreateChunk(vec3(0, 0, 0), 0, 0, 100, 100);
  Buffer(chunk);
}

TerrainChunk* TerrainManager::CreateChunk(vec3 pos, int noise_offset_x, int noise_offset_y, int size_x, int size_y)
{
  vector<vec3> vertices(size_x * size_y);
  vector<unsigned int> indices;

  vector<float> height_values(size_x * size_y);
  vector<float> moisture_values(size_x * size_y);
  vector<float> temperature_values(size_x * size_y);

  vector<vec3> normals;
  vector<vec2> uvs(size_x * size_y);

  fnSimplex->GenUniformGrid2D(height_values.data(), noise_offset_x, noise_offset_y, size_x, size_y, 0.02f, 1337);
  fnSimplex->GenUniformGrid2D(moisture_values.data(), noise_offset_x, noise_offset_y, size_x, size_y, 0.02f, 1337);
  fnSimplex->GenUniformGrid2D(temperature_values.data(), noise_offset_x, noise_offset_y, size_x, size_y, 0.02f, 1337);

  const int SIZE = 1;
  TerrainChunk* chunk = new TerrainChunk(size_x, size_y, SIZE, pos);

  // create chunk in local space, use transform matrix to apply offsetX/Y
  // each tile has 4 vertices

  // 1. fill simple array and adjastent tiles references
  for (int y = 0; y < size_y; y++) {
    for (int x = 0; x < size_x; x++) {
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

      bool is_last_row_vertices = (x == size_x - 1) || (y == size_y - 1);
      if (is_last_row_vertices) {
        continue;
      }

      TerrainTile* current_tile = &chunk->tiles[ind];
      current_tile->x0y0z = { x0, y0, height_values[ind] };
      current_tile->x0y1z = { x0, y1, height_values[ind_n] };
      current_tile->x1y0z = { x1, y0, height_values[ind_e] };
      current_tile->x1y1z = { x1, y1, height_values[ind_ne] };

      if (x == 0) {
        int prev_row_shift = (y - 1) * size_x;

        int ind_w = x0 - 1 + current_row_shift;
        int ind_sw = (x0 - 1) + prev_row_shift;
        int ind_s = x0 + prev_row_shift;
        int ind_se = (x0 + 1) + prev_row_shift;
        int ind_nw = (x0 - 1) + next_row_shift;

        AdjastentTiles& adj = current_tile->adjastent;

        bool is_east_border = x1 >= size_x;
        bool is_south_border = y0 == 0;
        bool is_west_border = x0 == 0;
        bool is_north_border = y1 >= size_y;

        adj.e = is_east_border ? nullptr : &chunk->tiles[ind_e];
        adj.se = is_east_border || is_south_border ? nullptr : &chunk->tiles[ind_se];
        adj.s = is_south_border ? nullptr : &chunk->tiles[ind_s];
        adj.w = is_west_border ? nullptr : &chunk->tiles[ind_w];
        adj.nw = is_west_border || is_north_border ? nullptr : &chunk->tiles[ind_nw];
        adj.n = is_north_border ? nullptr : &chunk->tiles[ind_n];
        adj.ne = is_north_border || is_east_border ? nullptr : &chunk->tiles[ind_ne];
        adj.sw = is_south_border || is_west_border ? nullptr : &chunk->tiles[ind_sw];
      }

      TerrainPixelValues& pixel_values = current_tile->pixel_values;
      pixel_values.height = (
        height_values[ind]
        + height_values[ind_e]
        + height_values[ind_ne]
        + height_values[ind_n]
       ) / 4;
      pixel_values.moisture = (
        moisture_values[ind]
        + moisture_values[ind_e]
        + moisture_values[ind_ne]
        + moisture_values[ind_n]
       ) / 4;
      pixel_values.temperature = (
        temperature_values[ind]
        + temperature_values[ind_e]
        + temperature_values[ind_ne]
        + temperature_values[ind_n]
       ) / 4;
    }
  }
  return chunk;
}

void TerrainManager::ReleaseBuffer(TerrainChunk* chunk) {
  buffer->ReleaseStorage(chunk->tiles[0].mesh_data);
}

void TerrainManager::Deactivate(TerrainChunk* chunk)
{
  draw_call_manager->DisableCommand(chunk->tiles[0].mesh_data);
}

void TerrainManager::Buffer(TerrainChunk* chunk) {
  // EACH VERTEX SHARED AMONG 1 - 4 TILES (CORNER 1 TILE, TOP/BOTTOM/LEFT/RIGHT BORDER - 2 TILES, OTHER - 4 TILES)

  for (int i = 0; i < chunk->tiles.size(); i++) {
    auto tile = &chunk->tiles[i];
    auto mesh_data = GetInstancedPlaneWithBaseMeshTransform(chunk, tile);
    mesh_data->transform = translate(mesh_data->transform, tile->x0y0z);
    buffer->BufferTransform(mesh_data);
    // TODO: assign blend textures
  }
}

MeshData* TerrainManager::GetInstancedPlaneWithBaseMeshTransform(
  TerrainChunk* chunk, TerrainTile* tile, unsigned int instance_count
  //, vec2& out_face_normal
) {
  auto existing = planes_for_instanced_meshes.find(chunk->tile_size);
  if (existing == planes_for_instanced_meshes.end()) {

    // TODO: for any number of vertices, not just hardcoded 2
    float half_size_x = (float)chunk->scale / 2;
    float half_size_y = (float)chunk->scale / 2;
    std::vector<vec3> vertices = {
        vec3(half_size_x,  half_size_y, 0.0f),  // top right
        vec3(half_size_x, -half_size_y, 0.0f),  // bottom right
        vec3(- half_size_x, -half_size_y, 0.0f),  // bottom left
        vec3(-half_size_x,  half_size_y, 0.0f)   // top left 
    };
    std::vector<unsigned int> indices = {
      0, 1, 3,  // first Triangle
      1, 2, 3   // second Triangle
    };

    // need to calculate vertex normal as mean of 4 faces normals
    // but we have instanced draw call, same normal for all tiles...
    // but transform mat4 should adjust each vertex normal to be equal to face normal
    // TODO: check that:
    //  expected to see seems between tiles and not smooth shading, each tile having constant light, no interpolation
    //  normal texture should make it a bit less visible though
    //out_face_normal = cross(vertices[0] - vertices[1], vertices[2] - vertices[3]);
    vec3 face_normal = cross(vertices[0] - vertices[1], vertices[2] - vertices[3]);
    vector<vec3> normals = {
      face_normal,
      face_normal,
      face_normal,
      face_normal
    };

    auto load_data = mesh_manager->CreateMesh(vertices, indices, normals);
    auto base_mesh = mesh_manager->CreateMeshData();

    base_mesh->transform = translate(mat4(1), chunk->pos);

    buffer->AllocateStorage(base_mesh, vertices.size(), indices.size());
    buffer->BufferMeshData(base_mesh, load_data);
    draw_call_manager->AddNewCommandToDrawCall(base_mesh, draw_call_manager->mesh_dc, instance_count);

    tile->mesh_data = base_mesh;

    planes_for_instanced_meshes[chunk->tile_size] = base_mesh;
    return planes_for_instanced_meshes[chunk->tile_size];

  }
  else {

    auto base_mesh = existing->second;
    auto instanced_mesh = mesh_manager->CreateInstancedMesh(base_mesh);
    instanced_mesh->transform = base_mesh->transform;
    draw_call_manager->SetInstanceCountToCommand(base_mesh, instance_count);
    draw_call_manager->AddNewInstanceSetInstanceId(instanced_mesh);
    return instanced_mesh;
  }
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
  //draw_call->buffer->ActivateCommandBuffer(draw_call->command_buffer);
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
  //draw_call->buffer->AddCommands(commands, draw_call->command_buffer);
  //CheckGLError();
  // -------- 1. END ---------------------------------------------------------------------
//}
