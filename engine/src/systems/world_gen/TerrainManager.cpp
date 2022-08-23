#include "TerrainManager.h"

void TerrainManager::CreateTerrain() {
  SystemsManager* systems_manager = SystemsManager::GetInstance();
  MeshManager* mesh_manager = systems_manager->mesh_manager;
  BufferManager* buffer_manager = systems_manager->buffer_manager;
  TextureManager* texture_manager = systems_manager->texture_manager;
  ShaderManager* shader_manager = systems_manager->shader_manager;
  MeshDataBufferConsumer* mesh_data_buffer = buffer_manager->mesh_data_buffer;

  int sizeX = 16;
  int sizeY = 16;

  vector<vec3> vertices(sizeX * sizeY);
  vector<unsigned int> indices;
  vector<float> noiseOutput(sizeX * sizeY);

  fnSimplex->GenUniformGrid2D(noiseOutput.data(), 0, 0, sizeX, sizeY, 0.02f, 1337);
  for (int y = 0; y < sizeY; y++) {
    for (int x = 0; x < sizeX; x++) {
      int ind = x + y * sizeY;
      vec3& vert = vertices[ind];
      vert.x = x;
      vert.y = y;
      vert.z = noiseOutput[ind];
      indices.push_back(ind);
      indices.push_back(ind + 1);
      indices.push_back(ind + 2);
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
  vector<shared_ptr<MeshLoadData>> load_data = { mesh_manager->CreateMesh(vertices, indices) };
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

  float light_power = 20.0;
  // add light for blinn_phong to view texture
  vector<Light*> light1 = { new Light{ {0,-1,0,0},
                       { 0.924208, 1.4625, 5.52104,0},
                       {light_power ,light_power ,light_power ,0},
                       {light_power ,light_power ,light_power ,0},
                       LightType::Point,
1.00000000,
0.699999988,
0.699999988,
                       0,
                       0 } };
  mesh_data_buffer->BufferLights(light1);
  // add command to drawcall
  vector<DrawElementsIndirectCommand> commands = { mesh->command };
  draw_call->command_count = commands.size();
  draw_call->buffer->AddCommands(commands, draw_call->command_buffer);
  CheckGLError();
  // -------- 1. END ---------------------------------------------------------------------
}
