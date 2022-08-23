#include "TerrainManager.h"

void TerrainManager::CreateTerrain() {
  SystemsManager* systems_manager = SystemsManager::GetInstance();
  MeshManager* mesh_manager = systems_manager->mesh_manager;
  BufferManager* buffer_manager = systems_manager->buffer_manager;
  TextureManager* texture_manager = systems_manager->texture_manager;
  ShaderManager* shader_manager = systems_manager->shader_manager;

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
  vector<shared_ptr<MeshLoadData>> load_data = { mesh_manager->CreateMesh(vertices, indices) };
  load_data[0]->tex_names = MaterialTexNames("checker1.png", EMPTY_STRING, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING);
  vector<MeshDataBase*> mesh = { mesh_manager->CreateMeshData(load_data[0].get()) };
  buffer_manager->mesh_data_buffer->
    BufferMeshData(mesh, load_data);
  buffer_manager->mesh_data_buffer->
    SetBaseMeshForInstancedCommand(mesh, load_data);
  buffer_manager->mesh_data_buffer->BufferTransform(
          (MeshData*)mesh[0]);

  texture_manager->LoadTextureArray(load_data[0]->tex_names);
  //mesh_data_buffer->BufferMeshTexture(
            //mesh);
  //mesh->texture->MakeResident();

  // -----------------------------------------------------------------------------------
  auto draw_call = new DrawCall();
  draw_call->shader = shader_manager->all_shaders->blinn_phong;
  draw_call->mode = GL_TRIANGLES;
  draw_call->buffer = systems_manager->buffer_manager->mesh_data_buffer;
  draw_call->command_buffer =
    draw_call->buffer->CreateCommandBuffer(BufferSettings::MAX_COMMAND);
  draw_call->buffer->ActivateCommandBuffer(draw_call->command_buffer);

  // draw_call2 = new DrawCall(*draw_call);
  ////draw_call2->shader = shaders.stencil;
  // draw_call2->command_buffer =
  // draw_call2->buffer->CreateCommandBuffer(command_buffer_size);

  systems_manager->renderer->AddDraw(draw_call, DrawOrder::MESH);
  // -----------------------------------------------------------------------------------
}
