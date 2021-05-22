#pragma once

#include "sausage.h"
#include "Structures.h"
#include "Logging.h"

using namespace std;

class BufferStorage {
private:
    constexpr int MAX_VERTEX = 1 000 000;
    constexpr int VERTEX_STORAGE_SIZE = MAX_VERTEX * sizeof(Vertex);
    
    constexpr int MAX_INDEX = 100 000;
    constexpr int INDEX_STORAGE_SIZE = MAX_INDEX * sizeof(unsigned int);
    
    constexpr int MAX_COMMAND = 10 000;
    constexpr int COMMAND_STORAGE_SIZE = MAX_COMMAND * sizeof(DrawElementsIndirectCommand);

    // UNIFORMS ///////////////////////////////
    constexpr int MAX_TRANSFORM = MAX_COMMAND;
    constexpr int TRANSFORM_STORAGE_SIZE = MAX_TRANSFORM * sizeof(mat4)

    constexpr int MAX_IDS = MAX_COMMAND;
    constexpr int IDS_STORAGE_SIZE = MAX_IDS * sizeof(MeshShaderIds)
    ////////////////////////////////////////////
public:

	unsigned int mesh_VAO;
    
    unsigned int vertex_buffer;
    Vertex* vertex_ptr;
    unsigned int vertex_total = 0;
    
    unsigned int index_buffer;
    unsigned int* index_ptr;
    unsigned index_total = 0;
    
    unsigned int command_buffer;
    DrawElementsIndirectCommand* command_ptr;
    unsigned int comand_total = 0;

    // UNIFORMS ///////////////////////////////
    unsigned int transform_buffer;
    mat4* transform_ptr;
    unsigned int transforms_total = 0;

    unsigned int ids_buffer;
    DrawElementsIndirectCommand* ids_ptr;
    unsigned int ids_total = 0;
    ////////////////////////////////////////////
	
    BufferStorage() {};
	~BufferStorage() {};

    void WaitGPU(GLsync* fence_sync) {
        GLbitfield waitFlags = 0;
        GLuint64 waitDuration = 0;
        while (true) {
            GLenum waitRet = glClientWaitSync(fence_sync, waitFlags, waitDuration);
            if (waitRet == GL_ALREADY_SIGNALED || waitRet == GL_CONDITION_SATISFIED) {
                return;
            }

            if (waitRet == GL_WAIT_FAILED) {
                LOG(string("WaitGPU wait sync returned status GL_WAIT_FAILED"));
                return;
            }

            // After the first time, need to start flushing, and wait for a looong time.
            waitFlags = GL_SYNC_FLUSH_COMMANDS_BIT;
            waitDuration = kOneSecondInNanoSeconds;
        }
        glDeleteSync(fence_sync)
    }

    /**
    * fence_sync created after all render ops issued for these buffers.
    */
    void BufferMeshData(
        vector<vector<Vertex>>& vertices,
        vector<vector<unsigned int>>& indices,
        vector<unsigned int>& draw_ids,
        vector<MeshShaderIds>& shader_ids,
        GLsync* fence_sync) {
        assert(vertices.size() == indices.size() && vertices.size() == draw_ids.size());
        if (fence_sync != nullptr) {
            WaitGPU(fence_sync);
        }
        for (int i = 0; i < vertices.size(); i++) {
            if (vertices[i].size() + vertex_total > MAX_VERTEX) {
                LOG(std::format("ERROR BufferMeshData allocation. vertices total=%i asked=%i max=%i;", vertex_total, vertices.size(), MAX_VERTEX));
                return;
            }
            if (indices[i].size() + index_total > MAX_INDEX) {
                LOG(std::format("ERROR BufferMeshData allocation. indices total=%i asked=%i max=%i;", index_total, vertices.size(), MAX_INDEX));
                return;
            }
            if (draw_ids[i].size() + command_total > MAX_COMMAND) {
                LOG(std::format("ERROR BufferMeshData allocation. commands total=%i asked=%i max=%i", command_total, draw_ids.size(), MAX_COMMAND));
                return;
            }
            if (shader_ids[i].size() + command_total > MAX_COMMAND) {
                LOG(std::format("ERROR BufferMeshData allocation. commands total=%i asked=%i max=%i", command_total, shader_ids.size(), MAX_COMMAND));
                return;
            }
            DrawElementsIndirectCommand command;
            command.baseVertex = vertex_total;
            command.count = indices.count();
            command.firstIndex = 0;
            command.baseInstance = draw_ids[i];
            command.instanceCount = 1;

            // copy to GPU
            memcpy(&vertex_ptr[vertex_total], &vertices[i].begin(), vertices[i].size() * sizeof(Vertex));
            memcpy(&index_ptr[index_total], &indices[i].begin(), indices[i].size() * sizeof(unsigned int));
            ids_ptr[ids_total] = shader_ids[i];
            command_ptr[comand_total] = command;

            // update totals
            vertex_total += vertices[i].size();
            index_total += indices[i].size();
            command_total += draw_ids[i].size();
            ids_total += shader_ids.size();
        }
        // ids_ptr is SSBO. call after SSBO write (GL_SHADER_STORAGE_BUFFER). https://www.khronos.org/opengl/wiki/Memory_Model#Incoherent_memory_access
        glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
    }
    void BufferTransforms(vector<mat4>& transforms, GLsync* fence_sync) {
        if (fence_sync != nullptr) {
            WaitGPU(fence_sync);
        }
        // TODO: remove from buffer transforms path. (updated frequently, avoid branching?)
        if (transforms.size() != comand_total) {
            LOG(std::format("ERROR BufferTransforms allocation. transforms size=%i must be equal to draws size=%i", transforms.size(), comand_total));
            return;
        }
        // TODO: remove from buffer transforms path. (updated frequently, avoid branching?)
        if (transforms.size() + command_total > MAX_TRANSFORM) {
            LOG(std::format("ERROR BufferTransforms allocation. transforms total=%i asked=%i max=%i", transforms_total, transforms.size(), MAX_TRANSFORM));
            return;
        }
        // call after SSBO write (GL_SHADER_STORAGE_BUFFER). https://www.khronos.org/opengl/wiki/Memory_Model#Incoherent_memory_access
        glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
    }
    void BufferTextureData() {

    }
    void InitMeshBuffers() {
        glGenBuffers(1, &vertex_buffer);
        glGenBuffers(1, &index_buffer);
        glGenBuffers(1, &command_buffer);
        glGenBuffers(1, &transform_buffer);

        const GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffer);
        glBufferStorage(GL_DRAW_INDIRECT_BUFFER, COMMAND_STORAGE_SIZE, NULL, flags);
        command_ptr = (DrawElementsIndirectCommand*)glMapBufferRange(GL_ARRAY_BUFFER, 0, COMMAND_STORAGE_SIZE, flags);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, INDEX_STORAGE_SIZE, NULL, flags);
        index_ptr = (unsigned int*)glMapBufferRange(GL_ARRAY_BUFFER, 0, INDEX_STORAGE_SIZE, flags);

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferStorage(GL_ARRAY_BUFFER, VERTEX_STORAGE_SIZE, NULL, flags);
        vertex_ptr = (Vertex*)glMapBufferRange(GL_ARRAY_BUFFER, 0, VERTEX_STORAGE_SIZE, flags);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, transform_buffer);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, TRANSFORM_STORAGE_SIZE, NULL, flags);
        transform_ptr = (mat4*)glMapBufferRange(GL_ARRAY_BUFFER, 0, TRANSFORMS_STORAGE_SIZE, flags);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ids_buffer);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, IDS_STORAGE_SIZE, NULL, flags);
        ids_ptr = (mat4*)glMapBufferRange(GL_ARRAY_BUFFER, 0, IDS_STORAGE_SIZE, flags);

    }
    void InitMeshVAO() {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        // pos
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
    }
    void Dispose() {
        glDisableVertexAttribArray(0);
        glDeleteVertexArrays(1, &mesh_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glDeleteBuffers(1, &vertex_buffer);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
        glDeleteBuffers(1, &index);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, transform_buffer);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glDeleteBuffers(1, &transform_buffer);

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffer);
        glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
        glDeleteBuffers(1, &command_buffer);
    }
};