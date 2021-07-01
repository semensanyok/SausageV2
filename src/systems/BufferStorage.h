#pragma once

#include "sausage.h"
#include "Structures.h"
#include "Logging.h"
#include "OpenGLHelpers.h"
#include "Texture.h"
#include "Settings.h"

using namespace std;
using namespace glm;
using namespace BufferSettings;


class BufferStorage {
private:
    const unsigned long VERTEX_STORAGE_SIZE = MAX_VERTEX * sizeof(Vertex);
    const unsigned long INDEX_STORAGE_SIZE = MAX_INDEX * sizeof(unsigned int);
    const unsigned long COMMAND_STORAGE_SIZE = MAX_COMMAND * sizeof(DrawElementsIndirectCommand);
    const unsigned long LIGHT_STORAGE_SIZE = MAX_LIGHTS * sizeof(Light);
    ///////////
    // UNIFORMS
    ///////////
    const unsigned long UNIFORMS_STORAGE_SIZE = sizeof(UniformData);
    const unsigned long TRANSFORM_OFFSET_STORAGE_SIZE = MAX_TRANSFORM_OFFSET * sizeof(unsigned int);
    const unsigned long TEXTURE_STORAGE_SIZE = MAX_TEXTURES * sizeof(GLuint64);
    
    const GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    const int UNIFORMS_LOC = 0;
    const int TEXTURE_UNIFORM_LOC = 1;
    const int LIGHTS_UNIFORM_LOC = 2;
    
    bool is_need_barrier = false;

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
    GLuint uniforms_buffer;
    GLuint texture_buffer;
    GLuint light_buffer;
    vector<GLuint> command_buffers;
    map<GLuint, DrawElementsIndirectCommand*> mapped_command_buffers;
    //////////////////////////
    // Mapped buffers pointers
    //////////////////////////
    Vertex* vertex_ptr;
    unsigned int* index_ptr;
    UniformData* uniforms_ptr;
    GLuint64* texture_ptr;
    Lights* light_ptr;

public:
    unsigned long meshes_total;
    unsigned long transforms_total;

    int id = -1;

    GLsync fence_sync = 0;

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
        fence_sync = 0;
    }
    void SyncGPUBufAndUnmap() {
        WaitGPU(fence_sync);
        if (is_need_barrier) {
            glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
            is_need_barrier = false;
        }
        for (auto& mapped_command_buffer : mapped_command_buffers) {
            // MUST unmap GL_DRAW_INDIRECT_BUFFER. GL_INVALID_OPERATION otherwise.
            if (!glUnmapNamedBuffer(mapped_command_buffer.first)) {
                CheckGLError();
            }
        }
        mapped_command_buffers.clear();
    }
    /**
    * fence_sync created after all render ops issued for these buffers.
    * 
    */
    void BufferMeshData(vector<shared_ptr<MeshLoadData>>& load_data, bool is_transform_used = true)
    {
        
        vector<MeshData*> instances;
        for (int i = 0; i < load_data.size(); i++) {
            auto raw = load_data[i].get();
            auto& mesh_data = raw->mesh_data;
            bool is_instance = mesh_data->base_mesh != nullptr;
            mesh_data->buffer = this;
            if (is_instance) {
                instances.push_back(mesh_data);
                continue;
            }
            auto& vertices = raw->vertices;
            auto& indices = raw->indices;
            // if offset initialized - reload data. (if vertices/indices size > existing - will corrupt other meshes)
            bool is_new_mesh = mesh_data->buffer_id < 0;

            bool is_vertex_offset_provided = mesh_data->vertex_offset >= 0;
            bool is_index_offset_provided = mesh_data->index_offset >= 0;
            if (!is_vertex_offset_provided) {
                mesh_data->vertex_offset = vertex_total;
            }
            if (!is_index_offset_provided) {
                mesh_data->index_offset = index_total;
            }

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
            if (is_new_mesh) {
                mesh_data->buffer_id = meshes_total++;
            }
            DrawElementsIndirectCommand& command = mesh_data->command;
            command.count = indices.size();
            command.instanceCount = raw->instance_count;
            command.firstIndex = mesh_data->index_offset;
            command.baseVertex = mesh_data->vertex_offset;
            command.baseInstance = mesh_data->buffer_id;
            // copy to GPU
            memcpy(&vertex_ptr[mesh_data->vertex_offset], vertices.data(), vertices.size() * sizeof(Vertex));
            memcpy(&index_ptr[mesh_data->index_offset], indices.data(), indices.size() * sizeof(unsigned int));
            if (mesh_data->texture != nullptr) {
                texture_ptr[mesh_data->buffer_id] = mesh_data->texture->texture_handle_ARB;
            }

            if (!is_vertex_offset_provided) {
                vertex_total += vertices.size();
            }
            if (!is_index_offset_provided) {
                index_total += indices.size();
            }
            if (is_transform_used) {
                mesh_data->transform_offset = transforms_total;
                if (is_new_mesh) {
                    transforms_total += command.instanceCount;
                }
            }
            if (mesh_data->armature != nullptr) {
                int num_bones = mesh_data->armature->num_bones;
                auto identity = mat4(1.0);
                BufferBoneTransform(mesh_data->armature->bones, identity, mesh_data->armature->num_bones);
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
    GLuint CreateCommandBuffer(unsigned int size) {
        GLuint command_buffer;
        // MUST unmap INDIRECT DRAW pointer after buffering. Hence - map on demand.
        glGenBuffers(1, &command_buffer);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffer);
        glBufferStorage(GL_DRAW_INDIRECT_BUFFER, size * sizeof(DrawElementsIndirectCommand), NULL, flags);
        command_buffers.push_back(command_buffer);
        CheckGLError();
        return command_buffer;
    }
    int AddCommands(vector<DrawElementsIndirectCommand>& active_commands, GLuint command_buffer, unsigned int size, int command_offset = -1) {
        
        int command_start = command_offset == -1 ? 0 : command_offset;
        auto command_ptr_iter = mapped_command_buffers.find(command_buffer);
        if (command_ptr_iter == mapped_command_buffers.end()) {
            // MUST unmap INDIRECT DRAW pointer after buffering. Hence - map on demand.
            auto command_ptr = (DrawElementsIndirectCommand*)glMapNamedBufferRange(command_buffer, 0, size * sizeof(DrawElementsIndirectCommand), flags);
            mapped_command_buffers[command_buffer] = command_ptr;
        }
        memcpy(&(mapped_command_buffers[command_buffer][command_start]), active_commands.data(), active_commands.size() * sizeof(DrawElementsIndirectCommand));
        return command_start;
    }
    int AddCommand(DrawElementsIndirectCommand& command, GLuint command_buffer, unsigned int size, int command_offset = -1) {
        
        int command_start = command_offset == -1 ? 0 : command_offset;
        auto command_ptr_iter = mapped_command_buffers.find(command_buffer);
        if (command_ptr_iter == mapped_command_buffers.end()) {
            // MUST unmap INDIRECT DRAW pointer after buffering. Hence - map on demand.
            auto command_ptr = (DrawElementsIndirectCommand*)glMapNamedBufferRange(command_buffer, 0, size * sizeof(DrawElementsIndirectCommand), flags);
            CheckGLError();
            mapped_command_buffers[command_buffer] = command_ptr;
        }
        mapped_command_buffers[command_buffer][command_start] = command;
        return command_start;
    }
    void BufferBoneTransform(Bone* bone, vector<mat4>& transforms, unsigned int num_bones = 1) {
        
        for (size_t i = 0; i < num_bones; i++)
        {
            BufferBoneTransform(&bone[i], transforms[i], 1);
        }
        is_need_barrier = true;
    }
    void BufferBoneTransform(Bone* bone, mat4& transform, unsigned int num_bones = 1) {
        
        for (size_t i = 0; i < num_bones; i++)
        {
          uniforms_ptr->bones_transforms[bone[i].id] = transform;
        }
        is_need_barrier = true;
    }
    void BufferTransform(MeshData* mesh_data) {
        
        uniforms_ptr->transforms[mesh_data->transform_offset + mesh_data->instance_id] = mesh_data->transform;
        if (mesh_data->instance_id == 0) {
            uniforms_ptr->transform_offset[mesh_data->buffer_id + mesh_data->instance_id] = mesh_data->transform_offset;
        }
        is_need_barrier = true;
    }
    void BufferTransform(vector<MeshData*>& mesh_data) {
        
        for (int i = 0; i < mesh_data.size(); i++) {
            BufferTransform(mesh_data[i]);
        }
        //memcpy(&mvp_ptr[command_total], mvps.data(), mvps.size() * sizeof(mat4));
        is_need_barrier = true;
    }
    void BufferLights(vector<Light*>& lights) {
        
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

        glGenBuffers(1, &index_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, INDEX_STORAGE_SIZE, NULL, flags);
        index_ptr = (unsigned int*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, INDEX_STORAGE_SIZE, flags);

        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferStorage(GL_ARRAY_BUFFER, VERTEX_STORAGE_SIZE, NULL, flags);
        vertex_ptr = (Vertex*)glMapBufferRange(GL_ARRAY_BUFFER, 0, VERTEX_STORAGE_SIZE, flags);

        glGenBuffers(1, &uniforms_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_buffer);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, UNIFORMS_STORAGE_SIZE, NULL, flags);
        uniforms_ptr = (UniformData*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, UNIFORMS_STORAGE_SIZE, flags);

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
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, texture_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, light_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

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
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, BoneIds));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, BoneWeights));
        
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, UNIFORMS_LOC, uniforms_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, TEXTURE_UNIFORM_LOC, texture_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, LIGHTS_UNIFORM_LOC, light_buffer);
    }
    void Dispose() {
        SyncGPUBufAndUnmap();
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
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_buffer);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glDeleteBuffers(1, &uniforms_buffer);
        CheckGLError();
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, texture_buffer);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glDeleteBuffers(1, &texture_buffer);
        CheckGLError();
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, light_buffer);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glDeleteBuffers(1, &light_buffer);
        CheckGLError();
        for (auto command_buffer : command_buffers) {
            glDeleteBuffers(1, &command_buffer);
        }
        CheckGLError();
    }
};
