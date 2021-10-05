#pragma once

#include "sausage.h"
#include "Settings.h"
#include "Structures.h"
#include "Logging.h"
#include "OpenGLHelpers.h"
#include "BufferStorage.h"
#include "BufferConsumer.h"
#include "MeshManager.h"
using namespace std;
using namespace glm;

class FontBufferConsumer : public BufferConsumer {
    MeshManager* mesh_manager;
    BufferMargins margins;
    unsigned long vertex_total = 0;
    unsigned long index_total = 0;
    unsigned long meshes_total = 0;
public:
    FontBufferConsumer(BufferStorage* buffer, MeshManager* mesh_manager) : 
        BufferConsumer(buffer, BufferType::FONT_BUFFERS),
        mesh_manager{ mesh_manager } {}
    ~FontBufferConsumer() {

    }
    // TODO:
    // track available buckets.
    MeshDataFontUI* BufferMeshDataUI(
        vector<vec3>& vertices,
        vector<unsigned int>& indices,
        vector<vec3>& colors,
        vector<vec2>& uvs,
        vec3 relative_position,
        Texture* font_data,
        bool is_transform_used = true)
    {
        shared_ptr<MeshLoadData> load_data = mesh_manager->CreateMesh(vertices, indices, colors, uvs);
        auto mesh = load_data.get()->mesh;
        mesh->name = "TextUI";
        mesh->vertex_offset = margins.start_vertex;
        mesh->index_offset = margins.start_index;
        mesh->transform = translate(mat4(1), relative_position);
        buffer->BufferMeshData(load_data, vertex_total, index_total, meshes_total,
            margins, SausageDefaults::DEFAULT_MESH_DATA_VECTOR, is_transform_used);
        
        mesh->texture = font_data;
        font_data->MakeResident();
        
        return (MeshDataFontUI*)mesh;
    }
    void BufferTransform() {

    }
    void BufferFontTextureHandle(Texture* texture) {
        buffer->BufferFontTextureHandle(texture);
    }
    void Init() {
        margins = buffer->RequestStorage(BufferSettings::Margins::FONT_VERTEX_PART, BufferSettings::Margins::FONT_INDEX_PART);
        vertex_total = margins.start_vertex;
        index_total = margins.start_index;
    }
    void Reset() {
        Init();
    }
};