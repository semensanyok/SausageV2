#pragma once

#include "sausage.h"
#include "Settings.h"
#include "Structures.h"
#include "Logging.h"
#include "OpenGLHelpers.h"
#include "BufferStorage.h"
using namespace std;
using namespace glm;

class MeshDataBufferConsumer {
	const BufferMargins margins;
	const BufferStorage* buffer;
    unsigned long vertex_total;
    unsigned long index_total;

    unsigned long meshes_total = 0;
public:
    MeshDataBufferConsumer(BufferStorage* buffer) {
        this->buffer = buffer;
        margins = buffer->RequestStorage(BufferSettings::Margins::MESH_DATA_VERTEX_PART, BufferSettings::Margins::MESH_DATA_INDEX_PART);
        vertex_total = margins.start_vertex;
        index_total = margins.start_index;
    }
	~MeshDataBufferConsumer() {
	}
    void Reset() {
        vertex_total = 0;
        index_total = 0;
        meshes_total = 0;
    };
    void BufferMeshData(vector<shared_ptr<MeshLoadData>>& load_data, bool is_transform_used = true)
    {
        vector<MeshData*> instances;
        for (int i = 0; i < load_data.size(); i++) {
            auto raw = load_data[i].get();
            auto& mesh = raw->mesh;
            bool is_instance = mesh->base_mesh != nullptr;
            mesh->buffer = this;
            if (is_instance) {
                instances.push_back(mesh);
                continue;
            }
            auto& vertices = raw->vertices;
            auto& indices = raw->indices;
            // if offset initialized - reload data. (if vertices/indices size > existing - will corrupt other meshes)
            bool is_new_mesh = mesh->buffer_id < 0;

            bool is_vertex_offset_provided = mesh->vertex_offset >= 0;
            bool is_index_offset_provided = mesh->index_offset >= 0;
            if (!is_vertex_offset_provided) {
                mesh->vertex_offset = vertex_total;
            }
            if (!is_index_offset_provided) {
                mesh->index_offset = index_total;
            }

            if (vertices.size() + mesh->vertex_offset > margins.end_vertex) {
                LOG((ostringstream() << "ERROR BufferMeshData allocation. vertices total=" << vertex_total << "asked=" << vertices.size()
                    << "max=" << margins.end_vertex << " vertex offset=" << mesh->vertex_offset).str());
                return;
            }
            if (indices.size() + mesh->index_offset > MAX_INDEX) {
                LOG((ostringstream() << "ERROR BufferMeshData allocation. indices total=" << index_total << "asked=" << indices.size()
                    << "max=" << margins.end_ << " index offset=" << mesh->index_offset).str());
                return;
            }
            if (is_new_mesh) {
                mesh->buffer_id = meshes_total++;
            }
            DrawElementsIndirectCommand& command = mesh->command;
            command.count = indices.size();
            command.instanceCount = raw->instance_count;
            command.firstIndex = mesh->index_offset;
            command.baseVertex = mesh->vertex_offset;
            command.baseInstance = mesh->buffer_id;
            // copy to GPU
            memcpy(&buffer->vertex_ptr[mesh->vertex_offset], vertices.data(), vertices.size() * sizeof(Vertex));
            memcpy(&buffer->index_ptr[mesh->index_offset], indices.data(), indices.size() * sizeof(unsigned int));
            if (mesh->texture != nullptr) {
                buffer->texture_ptr[mesh->buffer_id] = mesh->texture->texture_handle_ARB;
            }

            if (!is_vertex_offset_provided) {
                vertex_total += vertices.size();
            }
            if (!is_index_offset_provided) {
                index_total += indices.size();
            }
            if (is_transform_used) {
                mesh->transform_offset = transforms_total;
                if (is_new_mesh) {
                    transforms_total += command.instanceCount;
                }
            }
            if (mesh->armature != nullptr && mesh->armature != NULL) {
                int num_bones = mesh->armature->num_bones;
                if (mesh->armature->bones != NULL) {
                    auto identity = mat4(1);
                    BufferBoneTransform(mesh->armature->bones, identity, mesh->armature->num_bones);
                }
            }
        }
        for (auto& instance : instances) {
            instance->buffer_id = instance->base_mesh->buffer_id;
            instance->transform_offset = instance->base_mesh->transform_offset;
        }
        // ids_ptr is SSBO. call after SSBO write (GL_SHADER_STORAGE_BUFFER).
        is_need_barrier = true;
        CheckGLError();
    }
    void SetBaseMeshForInstancedCommand(vector<shared_ptr<MeshLoadData>>& new_meshes) {
        map<size_t, shared_ptr<MeshLoadData>> instanced_data_lookup;
        for (auto& mesh_ptr : new_meshes) {
            auto mesh = mesh_ptr.get();
            auto key = mesh->tex_names.Hash() + mesh->vertices.size() + mesh->indices.size();
            auto base_mesh_ptr = instanced_data_lookup.find(key);
            if (base_mesh_ptr == instanced_data_lookup.end()) {
                instanced_data_lookup[key] = mesh_ptr;
                continue;
            }
            auto base_mesh = (*base_mesh_ptr).second.get();
            auto& instance_count = base_mesh->instance_count;
            mesh->mesh->instance_id = instance_count++;
            mesh->mesh->base_mesh = base_mesh->mesh;
        }
    }
}