#pragma once

#include "sausage.h"
#include "Structures.h"
#include "Logging.h"
#include "OpenGLHelpers.h"

using namespace std;
using namespace glm;

class BufferStorage {
public:
//private:
    const int MAX_VERTEX = 1000000;
    const int VERTEX_STORAGE_SIZE = MAX_VERTEX * sizeof(Vertex);
    
    const int MAX_INDEX = 100000;
    const int INDEX_STORAGE_SIZE = MAX_INDEX * sizeof(unsigned int);
    
    const int MAX_COMMAND = 10;
    const int COMMAND_STORAGE_SIZE = MAX_COMMAND * sizeof(DrawElementsIndirectCommand);

    // UNIFORMS ///////////////////////////////
    const int MAX_TRANSFORM = MAX_COMMAND;
    const int TRANSFORM_STORAGE_SIZE = MAX_TRANSFORM * sizeof(mat4);

    const int MAX_TEXTURES = 100;
    const int TEXTURE_STORAGE_SIZE = MAX_TEXTURES * sizeof(GLuint64);
    ////////////////////////////////////////////
    const GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

    const int TRANSFORMS_UNIFORM_LOC = 0;
    const int TEXTURE_UNIFORM_LOC = 1;

    // MAPPED BUFFERS
    Vertex* vertex_ptr;
    unsigned int* index_ptr;
    DrawElementsIndirectCommand* command_ptr;
    mat4* transform_ptr;
    GLuint64* texture_ptr;
public:

	unsigned int mesh_VAO;
    
    GLuint vertex_buffer;
    unsigned int vertex_total = 0;
    
    GLuint index_buffer;
    unsigned index_total = 0;

    // UNIFORMS AND SSBO///////////////////////////////
    GLuint command_buffer;
    unsigned int command_total = 0;
    
    GLuint transform_buffer;
    unsigned int transforms_total = 0;

    GLuint texture_buffer;
    unsigned int textures_total = 0;
    ////////////////////////////////////////////
	
    BufferStorage() {};
	~BufferStorage() {};

    void WaitGPU(GLsync fence_sync) {
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
            waitDuration = 1000000000; // one second in nanoseconds
        }
        glDeleteSync(fence_sync);
    }

    /**
    * fence_sync created after all render ops issued for these buffers.
    */
    void BufferMeshData(
        vector<vector<Vertex>>& vertices,
        vector<vector<unsigned int>>& indices,
        vector<unsigned int>& draw_ids,
        vector<GLuint64>& texture_ids,
        GLsync fence_sync) {
        assert(vertices.size() == indices.size() && vertices.size() == draw_ids.size() && vertices.size() == texture_ids.size());
        if (fence_sync != nullptr) {
            WaitGPU(fence_sync);
        }

        // MUST unmap INDIRECT DRAW pointer after buffering. Hence - map on demand.
        command_ptr = (DrawElementsIndirectCommand*)glMapBufferRange(GL_DRAW_INDIRECT_BUFFER, command_total, (command_total + draw_ids.size()) * sizeof(DrawElementsIndirectCommand), flags);
        for (int i = 0; i < vertices.size(); i++) {
            if (vertices[i].size() + vertex_total > MAX_VERTEX) {
                LOG((ostringstream() << "ERROR BufferMeshData allocation. vertices total=" << vertex_total << "asked=" << vertices[i].size() << "max=" << MAX_VERTEX).str());
                return;
            }
            if (indices[i].size() + index_total > MAX_INDEX) {
                LOG((ostringstream() << "ERROR BufferMeshData allocation. indices total=" << index_total << "asked=" << indices[i].size() << "max=" << MAX_INDEX).str());
                return;
            }
            if (command_total >= MAX_COMMAND) {
                LOG((ostringstream() << "ERROR BufferMeshData allocation. commands total=" << command_total << "max=" << MAX_COMMAND).str());
                return;
            }
            if (textures_total >= MAX_TEXTURES) {
                LOG((ostringstream() << "ERROR BufferMeshData allocation. textures total=" << textures_total << "max=" << MAX_TEXTURES).str());
                return;
            }
            DrawElementsIndirectCommand* command = &command_ptr[command_total];
            command->count = indices[i].size();
            command->instanceCount = 1;
            command->firstIndex = 0;
            command->baseVertex = vertex_total;
            command->baseInstance = draw_ids[i];

            // copy to GPU
            memcpy(&vertex_ptr[vertex_total], &*vertices[i].begin(), vertices[i].size() * sizeof(Vertex));
            memcpy(&index_ptr[index_total], &*indices[i].begin(), indices[i].size() * sizeof(unsigned int));
            texture_ptr[textures_total] = texture_ids[i];

            vertex_total += vertices[i].size();
            index_total += indices[i].size();
            textures_total++;
            command_total++;
        }
        // ids_ptr is SSBO. call after SSBO write (GL_SHADER_STORAGE_BUFFER).
        glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
        // MUST unmap GL_DRAW_INDIRECT_BUFFER. GL_INVALID_OPERATION otherwise.
        if (!glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER)) {
            CheckGLError();
        }
        CheckGLError();
    }

    void BufferTransforms(vector<mat4>& transforms, GLsync fence_sync) {
        if (fence_sync != nullptr) {
            WaitGPU(fence_sync);
        }
        // TODO: remove from buffer transforms path. (updated frequently)
        if (transforms.size() != command_total) {
            LOG((ostringstream() << "ERROR BufferTransforms allocation. transforms size=" << transforms.size() <<"must be equal to draws size=" << command_total).str());
            return;
        }
        memcpy(&transform_ptr[transforms_total], &*transforms.begin(), transforms.size() * sizeof(mat4));
        // call after SSBO write (GL_SHADER_STORAGE_BUFFER).
        glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
        transforms_total += transforms.size();
    }
    void InitMeshBuffers() {
        glGenVertexArrays(1, &mesh_VAO);
        glBindVertexArray(mesh_VAO); // MUST be bound before glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffer).

        // MUST unmap INDIRECT DRAW pointer after buffering. Hence - map on demand.
        glGenBuffers(1, &command_buffer);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffer);
        glBufferStorage(GL_DRAW_INDIRECT_BUFFER, COMMAND_STORAGE_SIZE, NULL,  flags);

        glGenBuffers(1, &index_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, INDEX_STORAGE_SIZE, NULL, flags);
        index_ptr = (unsigned int*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, INDEX_STORAGE_SIZE, flags);

        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferStorage(GL_ARRAY_BUFFER, VERTEX_STORAGE_SIZE, NULL, flags);
        vertex_ptr = (Vertex*)glMapBufferRange(GL_ARRAY_BUFFER, 0, VERTEX_STORAGE_SIZE, flags);

        glGenBuffers(1, &transform_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, transform_buffer);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, TRANSFORM_STORAGE_SIZE, NULL, flags);
        transform_ptr = (mat4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, TRANSFORM_STORAGE_SIZE, flags);

        glGenBuffers(1, &texture_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, texture_buffer);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, TEXTURE_STORAGE_SIZE, NULL, flags);
        texture_ptr = (GLuint64*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, TEXTURE_STORAGE_SIZE, flags);
    }

    void BindMeshVAOandBuffers() {
        // !WARNING. binding buffer after glVertexAttribPointer lead to hardware crash.
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, transform_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, texture_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffer);
        
        glBindVertexArray(mesh_VAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
        
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, TRANSFORMS_UNIFORM_LOC, transform_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, TEXTURE_UNIFORM_LOC, texture_buffer);
    }
    void Dispose() {
        glDisableVertexAttribArray(0);
        glDeleteVertexArrays(1, &mesh_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glDeleteBuffers(1, &vertex_buffer);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
        glDeleteBuffers(1, &index_buffer);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, transform_buffer);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glDeleteBuffers(1, &transform_buffer);

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffer);
        glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
        glDeleteBuffers(1, &command_buffer);
    }
};
