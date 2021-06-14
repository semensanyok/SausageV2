#pragma once

#include "sausage.h"
#include "Structures.h"
#include "Logging.h"
#include "OpenGLHelpers.h"

using namespace std;
using namespace glm;

class BufferStorage {
private:
    const int MAX_VERTEX = 1000000;
    const int VERTEX_STORAGE_SIZE = MAX_VERTEX * sizeof(Vertex);
    
    const int MAX_INDEX = 100000;
    const int INDEX_STORAGE_SIZE = MAX_INDEX * sizeof(unsigned int);
    
    const int MAX_COMMAND = 1000;
    const int COMMAND_STORAGE_SIZE = MAX_COMMAND * sizeof(DrawElementsIndirectCommand);
    
    const int MAX_LIGHTS = 1000;
    const int LIGHT_STORAGE_SIZE = MAX_LIGHTS * sizeof(Light);
    ///////////
    // UNIFORMS
    ///////////
    const int MAX_MVP = MAX_COMMAND;
    const int MVP_STORAGE_SIZE = MAX_MVP * sizeof(mat4);

    const int MAX_TEXTURES = 100;
    const int TEXTURE_STORAGE_SIZE = MAX_TEXTURES * sizeof(GLuint64);
    
    const GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    const int MVP_UNIFORM_LOC = 0;
    const int TEXTURE_UNIFORM_LOC = 1;
    const int LIGHTS_UNIFORM_LOC = 2;
    
    bool is_need_barrier = false;
    bool is_cmd_buffer_mapped = false;

	///////////
	/// Buffers
    ///////////
	unsigned int mesh_VAO;
    
    GLuint vertex_buffer;
    unsigned int vertex_total = 0;
    
    GLuint index_buffer;
    unsigned index_total = 0;
    /////////////////////
    // UNIFORMS AND SSBO
    /////////////////////
    GLuint mvp_buffer;
    GLuint command_buffer;
    unsigned int command_total = 0;

    GLuint texture_buffer;
    unsigned int textures_total = 0;
    GLuint light_buffer;
    //////////////////////////
    // Mapped buffers pointers
    //////////////////////////
    Vertex* vertex_ptr;
    unsigned int* index_ptr;
    DrawElementsIndirectCommand* command_ptr;
    mat4* mvp_ptr;
    GLuint64* texture_ptr;
    Lights* light_ptr;
public:
    int id = -1;

    GLsync fence_sync = 0;

    vector<DrawElementsIndirectCommand> active_commands;
    unsigned int active_commands_to_render = 0;
	
    bool bind_draw_buffer = true;
    BufferStorage() {
        static int count = 0;
        id = count++;
    };
	~BufferStorage() {};
    void Reset() {
        fence_sync = 0;
        active_commands_to_render = 0;
        vertex_total = 0;
        index_total = 0;
        command_total = 0;
        textures_total = 0;
    };
    bool operator<(const BufferStorage& other)
    {
        return id < other.id;
    }
    bool operator==(const BufferStorage& other)
    {
        return id == other.id;
    }

    void WaitGPU(GLsync fence_sync, const source_location& location = source_location::current()) {
        if (fence_sync == 0) {
            return;
        }
        GLbitfield waitFlags = 0;
        GLuint64 waitDuration = 0;
        while (true) {
            GLenum waitRet = glClientWaitSync(fence_sync, waitFlags, waitDuration);
            if (waitRet == GL_ALREADY_SIGNALED || waitRet == GL_CONDITION_SATISFIED) {
                return;
            }

            if (waitRet == GL_WAIT_FAILED) {
                LOG((ostringstream() << "WaitGPU wait sync returned status GL_WAIT_FAILED at" << location.file_name() << "("
                    << location.line() << ":"
                    << location.column() << ")#"
                    << location.function_name()).str());
                return;
            }

            // After the first time, need to start flushing, and wait for a looong time.
            waitFlags = GL_SYNC_FLUSH_COMMANDS_BIT;
            waitDuration = 1000000000; // one second in nanoseconds
        }
        glDeleteSync(fence_sync);
    }
    void BarrierIfChangeAndUnmap() {
        if (is_need_barrier) {
            glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
            is_need_barrier = false;
        }
        if (is_cmd_buffer_mapped) {
            // MUST unmap GL_DRAW_INDIRECT_BUFFER. GL_INVALID_OPERATION otherwise.
            if (!glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER)) {
                CheckGLError();
            }
            is_cmd_buffer_mapped = false;
        }
    }
    /**
    * fence_sync created after all render ops issued for these buffers.
    * 
    */
    void BufferMeshData(
        vector<vector<Vertex>>& vertices,
        vector<vector<unsigned int>>& indices,
        vector<MeshData>& mesh_data,
        vector<GLuint64>& texture_handles) {
        assert(vertices.size() == indices.size() && vertices.size() == mesh_data.size() && vertices.size() == texture_handles.size());
        WaitGPU(fence_sync);
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
            //***Return commands. perform hierarchical occlusion culling and then call multidraw with array of commands. (or buffer gpu for several frames, till camera exits 'view cell'
            //DrawElementsIndirectCommand command; = &command_ptr[command_total];
            //command->count = indices[i].size();
            //command->instanceCount = 1;
            //command->firstIndex = 0;
            //command->baseVertex = vertex_total;
            //command->baseInstance = mesh_data[i].id;

            DrawElementsIndirectCommand command;
            command.count = indices[i].size();
            command.instanceCount = 1;
            command.firstIndex = index_total;
            command.baseVertex = vertex_total;
            command.baseInstance = command_total;
            mesh_data[i].command = command;

            // copy to GPU
            memcpy(&vertex_ptr[vertex_total], vertices[i].data(), vertices[i].size() * sizeof(Vertex));
            memcpy(&index_ptr[index_total], indices[i].data(), indices[i].size() * sizeof(unsigned int));
            auto handle = texture_handles[i];
            if (handle != 0) {
                texture_ptr[textures_total] = handle;
            }
            vertex_total += vertices[i].size();
            index_total += indices[i].size();
            textures_total++;
            command_total++;
        }
        // ids_ptr is SSBO. call after SSBO write (GL_SHADER_STORAGE_BUFFER).
        is_need_barrier = true;
        CheckGLError();
    }
    int AddCommands(vector<DrawElementsIndirectCommand>& active_commands) {
    // todo: compare two
    //  to render for several frames, till camera exits 'view cell'
        if (bind_draw_buffer) {
            WaitGPU(fence_sync);
            if (!is_cmd_buffer_mapped) {
                // MUST unmap INDIRECT DRAW pointer after buffering. Hence - map on demand.
                glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffer);
                command_ptr = (DrawElementsIndirectCommand*)glMapBufferRange(GL_DRAW_INDIRECT_BUFFER, 0, active_commands.size() * sizeof(DrawElementsIndirectCommand), flags);
                CheckGLError();
                is_cmd_buffer_mapped = true;
            }
            memcpy(command_ptr, active_commands.data(), active_commands.size() * sizeof(DrawElementsIndirectCommand));
        }
    // to call multidraw with array of commands each frame.
        else {
            this->active_commands = active_commands;
        }
        int command_sequence_start_offset = active_commands_to_render;
        active_commands_to_render += active_commands.size();
        return command_sequence_start_offset;
    }
    
    void BufferTransform(MeshData& mesh_data, mat4& transform) {
        WaitGPU(fence_sync);
        if (mesh_data.id > command_total) {
            LOG((ostringstream() << "ERROR BufferMvp. offset=" << mesh_data.id << " not valid. expected less then command_total=" << command_total).str());
            return;
        }
        mvp_ptr[mesh_data.id] = transform;
        is_need_barrier = true;
    }
    void BufferTransform(vector<MeshData>& mesh_data, vector<mat4>& transform) {
        WaitGPU(fence_sync);
        if (transform.size() != mesh_data.size()) {
            LOG((ostringstream() << "ERROR BufferMvps. mesh_data param size=" << mesh_data.size() << "must be equal to mvps param size=" << transform.size()).str());
            return;
        }
        if (mesh_data.size() != command_total) {
            LOG((ostringstream() << "ERROR BufferMvps. mesh_data size=" << mesh_data.size() <<"must be equal to draws size=" << command_total).str());
            return;
        }
        for (int i = 0; i < transform.size(); i++) {
            mvp_ptr[mesh_data[i].id] = transform[i];
        }
        //memcpy(&mvp_ptr[command_total], mvps.data(), mvps.size() * sizeof(mat4));
        is_need_barrier = true;
    }
    void BufferLights(vector<Light>& lights) {
        WaitGPU(fence_sync);
        if (lights.size() > MAX_LIGHTS) {
            LOG((ostringstream() << "ERROR BufferLights. max lights buffer size=" << MAX_LIGHTS << " requested=" << lights.size()).str());
            return;
        }
        light_ptr->num_lights = lights.size();
        memcpy(light_ptr->lights, lights.data(), lights.size() * sizeof(Light));
        is_need_barrier = true;
    }
    void InitMeshBuffers() {
        glGenVertexArrays(1, &mesh_VAO);
        glBindVertexArray(mesh_VAO); // MUST be bound before glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffer).

        // MUST unmap INDIRECT DRAW pointer after buffering. Hence - map on demand.
        if (bind_draw_buffer) {
            glGenBuffers(1, &command_buffer);
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffer);
            glBufferStorage(GL_DRAW_INDIRECT_BUFFER, COMMAND_STORAGE_SIZE, NULL, flags);
        }

        glGenBuffers(1, &index_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, INDEX_STORAGE_SIZE, NULL, flags);
        index_ptr = (unsigned int*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, INDEX_STORAGE_SIZE, flags);

        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferStorage(GL_ARRAY_BUFFER, VERTEX_STORAGE_SIZE, NULL, flags);
        vertex_ptr = (Vertex*)glMapBufferRange(GL_ARRAY_BUFFER, 0, VERTEX_STORAGE_SIZE, flags);

        glGenBuffers(1, &mvp_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mvp_buffer);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, MVP_STORAGE_SIZE, NULL, flags);
        mvp_ptr = (mat4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, MVP_STORAGE_SIZE, flags);

        glGenBuffers(1, &texture_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, texture_buffer);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, TEXTURE_STORAGE_SIZE, NULL, flags);
        texture_ptr = (GLuint64*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, TEXTURE_STORAGE_SIZE, flags);

        glGenBuffers(1, &light_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, light_buffer);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, LIGHT_STORAGE_SIZE, NULL, flags);
        light_ptr = (Lights*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, LIGHT_STORAGE_SIZE, flags);
    }

    void BindMeshVAOandBuffers() {
        // !WARNING. binding buffer after glVertexAttribPointer lead to hardware crash.
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mvp_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, texture_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, light_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        if (bind_draw_buffer) {
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffer);
        }
        
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
        
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, MVP_UNIFORM_LOC, mvp_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, TEXTURE_UNIFORM_LOC, texture_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, LIGHTS_UNIFORM_LOC, light_buffer);
    }
    void Dispose() {
        WaitGPU(fence_sync);
        glDisableVertexAttribArray(0);
        glDeleteVertexArrays(1, &mesh_VAO);
        CheckGLError();
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glDeleteBuffers(1, &vertex_buffer);
        CheckGLError();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
        glDeleteBuffers(1, &index_buffer);
        CheckGLError();
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mvp_buffer);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glDeleteBuffers(1, &mvp_buffer);
        CheckGLError();
        if (bind_draw_buffer) {
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffer);
            if (is_cmd_buffer_mapped) {
                glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
            }
            glDeleteBuffers(1, &command_buffer);
        }
        CheckGLError();
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, texture_buffer);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glDeleteBuffers(1, &texture_buffer);
        CheckGLError();
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, light_buffer);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glDeleteBuffers(1, &light_buffer);
        CheckGLError();
    }
};
