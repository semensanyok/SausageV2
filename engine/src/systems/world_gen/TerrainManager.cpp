#include "TerrainManager.h"

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
    for (int x = 0; x < sizeX; x+=1) {
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
  auto draw_call = new DrawCall();
  draw_call->shader = shader_manager->all_shaders->blinn_phong;
  draw_call->mode = GL_TRIANGLES;
  draw_call->buffer = mesh_data_buffer;
  draw_call->command_buffer =
    draw_call->buffer->CreateCommandBuffer(BufferSettings::MAX_COMMAND);
  draw_call->buffer->ActivateCommandBuffer(draw_call->command_buffer);
  systems_manager->renderer->AddDraw(draw_call, DrawOrder::MESH);

  // load data
  vector<shared_ptr<MeshLoadData>> load_data = { mesh_manager->CreateMesh(vertices, indices, normals, uvs) };
  load_data[0]->tex_names = MaterialTexNames("checker1.png", EMPTY_STRING, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING);
  vector<MeshDataBase*> meshes = { mesh_manager->CreateMeshData(load_data[0].get()) };
  auto mesh = (MeshData*)meshes[0];

  // buffer data
  mesh_data_buffer->
    BufferMeshData(meshes, load_data);
  mesh_data_buffer->
    SetBaseMeshForInstancedCommand(meshes, load_data);
  mesh_data_buffer->BufferTransform(mesh);

  // load texture to buffer
  mesh->texture = texture_manager->LoadTextureArray(load_data[0]->tex_names);
  mesh_data_buffer->BufferMeshTexture(mesh);
  mesh->texture->MakeResident();

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
