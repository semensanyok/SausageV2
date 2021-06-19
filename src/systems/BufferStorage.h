#pragma once

#include "sausage.h"
#include "Structures.h"
#include "Logging.h"
#include "OpenGLHelpers.h"
#include "Texture.h"

using namespace std;
using namespace glm;

class BufferStorage {
private:
    const unsigned long VERTEX_STORAGE_SIZE = MAX_VERTEX * sizeof(Vertex);
    const unsigned long INDEX_STORAGE_SIZE = MAX_INDEX * sizeof(unsigned int);
    const unsigned long COMMAND_STORAGE_SIZE = MAX_COMMAND * sizeof(DrawElementsIndirectCommand);
    const unsigned long LIGHT_STORAGE_SIZE = MAX_LIGHTS * sizeof(Light);
    ///////////
    // UNIFORMS
    ///////////
    const unsigned long TRANSFORM_STORAGE_SIZE = MAX_TRANSFORM * sizeof(mat4);
    const unsigned long TRANSFORM_OFFSET_STORAGE_SIZE = MAX_TRANSFORM_OFFSET * sizeof(unsigned int);
    const unsigned long TEXTURE_STORAGE_SIZE = MAX_TEXTURES * sizeof(GLuint64);
    
    const GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    const int TRANSFORM_UNIFORM_LOC = 0;
    const int TRANSFORM_OFFSET_UNIFORM_LOC = 1;
    const int TEXTURE_UNIFORM_LOC = 2;
    const int LIGHTS_UNIFORM_LOC = 3;
    
    bool is_need_barrier = false;
    bool is_cmd_buffer_mapped = false;

    unsigned long vertex_total = 0;
    unsigned long index_total = 0;
    unsigned int textures_total = 0;
    
	///////////
	/// Buffers
    ///////////
    GLuint mesh_VAO;
    GLuint vertex_buffer;
    GLuint index_buffer;
    /////////////////////
    // UNIFORMS AND SSBO
    /////////////////////
    GLuint transform_buffer;
    GLuint transform_offset_buffer;
    GLuint command_buffer;
    GLuint texture_buffer;
    GLuint light_buffer;
    //////////////////////////
    // Mapped buffers pointers
    //////////////////////////
    Vertex* vertex_ptr;
    unsigned int* index_ptr;
    DrawElementsIndirectCommand* command_ptr;
    mat4* transform_ptr;
    unsigned int* transform_offset_ptr;
    GLuint64* texture_ptr;
    Lights* light_ptr;

public:
    unsigned long meshes_total;
    unsigned long transforms_total;

    static const unsigned long MAX_VERTEX = 1000000;
    static const unsigned long MAX_INDEX = 100000;
    static const unsigned long MAX_COMMAND = 1000;
    static const unsigned long MAX_TRANSFORM = MAX_COMMAND;
    static const unsigned long MAX_TRANSFORM_OFFSET = MAX_TRANSFORM * 10;
    static const unsigned long MAX_TEXTURES = MAX_COMMAND;
    static const unsigned long MAX_LIGHTS = 1000;
    
    int id = -1;

    GLsync fence_sync = 0;

    vector<DrawElementsIndirectCommand> active_commands;
    int active_commands_to_render = 0;
	
    bool bind_draw_buffer = true;
    BufferStorage() {
        static int count = 0;
        id = count++;

        meshes_total = 0;
        transforms_total = 0;
    };
	~BufferStorage() {};
    void Reset() {
        fence_sync = 0;
        active_commands_to_render = 0;
        vertex_total = 0;
        index_total = 0;
        meshes_total = 0;
        transforms_total = 0;
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
    void BufferMeshData(vector<shared_ptr<MeshLoadData>>& load_data)
    {
        WaitGPU(fence_sync);

        for (int i = 0; i < load_data.size(); i++) {
            auto raw = load_data[i].get();
            auto& mesh_data = raw->mesh_data;
            mesh_data->buffer = this;
            auto& vertices = raw->vertices;
            auto& indices = raw->indices;
            // if offset initialized - reload data. (if vertices/indices size > existing - will corrupt other meshes)
            mesh_data->vertex_offset = mesh_data->vertex_offset == -1 ? vertex_total : mesh_data->vertex_offset;
            mesh_data->index_offset = mesh_data->index_offset == -1 ? index_total : mesh_data->index_offset;
            if (vertices.size() + mesh_data->vertex_offset > MAX_VERTEX) {
                LOG((ostringstream() << "ERROR BufferMeshData allocation. vertices total=" << vertex_total << "asked=" << vertices.size()
                    << "max=" << MAX_VERTEX << " vertex offset=" << mesh_data->vertex_offset).str());
                return;
            }
            if (indices.size() + mesh_data->index_offset > MAX_INDEX) {
                LOG((ostringstream() << "ERROR BufferMeshData allocation. indices total=" << index_total << "asked=" << indices.size()
                    << "max=" << MAX_INDEX << " index offset=" << mesh_data->index_offset).str());
                return;
            }
            mesh_data->buffer_id = meshes_total++;
            mesh_data->transform_offset = transforms_total;
            DrawElementsIndirectCommand& command = mesh_data->command;
            command.count = indices.size();
            command.instanceCount = raw->instance_count;
            command.firstIndex = index_total;
            command.baseVertex = vertex_total;
            command.baseInstance = mesh_data->buffer_id;
            transforms_total += command.instanceCount;

            // copy to GPU
            memcpy(&vertex_ptr[mesh_data->vertex_offset], vertices.data(), vertices.size() * sizeof(Vertex));
            memcpy(&index_ptr[mesh_data->index_offset], indices.data(), indices.size() * sizeof(unsigned int));
            if (mesh_data->texture != nullptr) {
                texture_ptr[mesh_data->buffer_id] = mesh_data->texture->texture_handle_ARB;
            }
            if (mesh_data->vertex_offset == vertex_total) {
                vertex_total += vertices.size();
                index_total += indices.size();
            }
        }
        // ids_ptr is SSBO. call after SSBO write (GL_SHADER_STORAGE_BUFFER).
        is_need_barrier = true;
        CheckGLError();
    }
    int AddCommands(vector<DrawElementsIndirectCommand>& active_commands, int command_offset = -1) {
        // todo: compare two
        //  to render for several frames, till camera exits 'view cell'
        int& command_start = command_offset == -1 ? active_commands_to_render : command_offset;
        if (bind_draw_buffer) {
            if (command_offset > active_commands_to_render) {
                LOG((ostringstream() << "ERROR AddCommands. command_offset="<<command_offset<<" must be less or eq to active commands="<<active_commands_to_render).str());
                return 0;
            }
            if (command_start + active_commands.size() > MAX_COMMAND) {
                LOG((ostringstream() << "ERROR AddCommands. resuested=" << active_commands.size() << "max=" << MAX_COMMAND << "offset=" << command_start).str());
                return 0;
            }
            WaitGPU(fence_sync);
            if (!is_cmd_buffer_mapped) {
                // MUST unmap INDIRECT DRAW pointer after buffering. Hence - map on demand.
                glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffer);
                command_ptr = (DrawElementsIndirectCommand*)glMapBufferRange(GL_DRAW_INDIRECT_BUFFER, 0, COMMAND_STORAGE_SIZE, flags);
                CheckGLError();
                is_cmd_buffer_mapped = true;
            }
            memcpy(&command_ptr[command_start], active_commands.data(), active_commands.size() * sizeof(DrawElementsIndirectCommand));
        }
        // to call multidraw with array of commands each frame.
        else {
            this->active_commands = active_commands;
        }
        if (command_offset == -1) {
            active_commands_to_render += active_commands.size();
        }
        return command_start;
    }
    int AddCommand(DrawElementsIndirectCommand* command, int command_offset = -1) {
        // todo: compare two
        //  to render for several frames, till camera exits 'view cell'
        int& command_start = command_offset == -1 ? active_commands_to_render : command_offset;
        if (bind_draw_buffer) {
            if (command_offset > active_commands_to_render) {
                LOG((ostringstream() << "ERROR AddCommands. command_offset=" << command_offset << " must be less or eq to active commands=" << active_commands_to_render).str());
                return 0;
            }
            if (command_start > MAX_COMMAND) {
                LOG((ostringstream() << "ERROR AddCommand. max=" << MAX_COMMAND << "offset=" << command_start << "allocated=" << active_commands_to_render).str());
                return 0;
            }
            WaitGPU(fence_sync);
            if (!is_cmd_buffer_mapped) {
                // MUST unmap INDIRECT DRAW pointer after buffering. Hence - map on demand.
                glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffer);
                command_ptr = (DrawElementsIndirectCommand*)glMapBufferRange(GL_DRAW_INDIRECT_BUFFER, 0, COMMAND_STORAGE_SIZE, flags);
                CheckGLError();
                is_cmd_buffer_mapped = true;
            }
            command_ptr[command_start] = *command;
        }
        // to call multidraw with array of commands each frame.
        else {
            this->active_commands = active_commands;
        }
        if (command_offset == -1) {
            active_commands_to_render += active_commands.size();
        }
        return command_start;
    }
    
    void BufferTransform(MeshData* mesh_data) {
        WaitGPU(fence_sync);
        transform_ptr[mesh_data->transform_offset + mesh_data->instance_id] = mesh_data->transform;
        transform_offset_ptr[mesh_data->buffer_id + mesh_data->instance_id] = mesh_data->transform_offset;
        is_need_barrier = true;
    }
    void BufferTransform(vector<MeshData*>& mesh_data) {
        WaitGPU(fence_sync);
        for (int i = 0; i < mesh_data.size(); i++) {
            transform_ptr[mesh_data[i]->transform_offset + mesh_data[i]->instance_id] = mesh_data[i]->transform;
            transform_offset_ptr[mesh_data[i]->buffer_id + mesh_data[i]->instance_id] = mesh_data[i]->transform_offset;
        }
        //memcpy(&mvp_ptr[command_total], mvps.data(), mvps.size() * sizeof(mat4));
        is_need_barrier = true;
    }
    void BufferLights(vector<Light*>& lights) {
        WaitGPU(fence_sync);
        if (lights.size() > MAX_LIGHTS) {
            LOG((ostringstream() << "ERROR BufferLights. max lights buffer size=" << MAX_LIGHTS << " requested=" << lights.size()).str());
            return;
        }
        light_ptr->num_lights = lights.size();
        for (int i = 0; i < lights.size();i++) {
            light_ptr->lights[i] = *lights[i];
            //memcpy(light_ptr->lights, lights.data(), lights.size() * sizeof(Light));
        }
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

        glGenBuffers(1, &transform_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, transform_buffer);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, TRANSFORM_STORAGE_SIZE, NULL, flags);
        transform_ptr = (mat4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, TRANSFORM_STORAGE_SIZE, flags);

        glGenBuffers(1, &transform_offset_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, transform_offset_buffer);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, TRANSFORM_OFFSET_STORAGE_SIZE, NULL, flags);
        transform_offset_ptr = (unsigned int*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, TRANSFORM_OFFSET_STORAGE_SIZE, flags);

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
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, transform_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, transform_offset_buffer);
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
        
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, TRANSFORM_UNIFORM_LOC, transform_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, TRANSFORM_OFFSET_UNIFORM_LOC, transform_offset_buffer);
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
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, transform_buffer);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glDeleteBuffers(1, &transform_buffer);
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
