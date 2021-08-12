#pragma once

#include "../sausage.h"
#include "../Settings.h"
#include "../Structures.h"
#include "../Logging.h"
#include "../OpenGLHelpers.h"
#include "../Texture.h"

using namespace std;
using namespace glm;
using namespace BufferSettings;

//struct DataRangeLock {
//    unsigned int vertex_begin;
//    unsigned int vertex_end;
//    unsigned int index_begin;
//    unsigned int index_end;
//    unsigned int transform_begin;
//    unsigned int transform_end;
// 
//    mutex data_mutex;
//    condition_variable is_vertex_buffer_mapped;
//    bool is_mapped;
//};

class BufferStorage {
    friend class MeshDataBufferConsumer;
    friend class FontBufferConsumer;
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

    unsigned long vertex_start = 0;
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
    vector<CommandBuffer*> command_buffers;
    map<GLuint, CommandBuffer*> mapped_command_buffers;
    //////////////////////////
    // Mapped buffers pointers
    //////////////////////////
    Vertex* vertex_ptr;
    unsigned int* index_ptr;
    UniformData* uniforms_ptr;
    GLuint64* texture_ptr;
    Lights* light_ptr;

    float allocated_percent_vertex;
    float allocated_percent_index;
public:
    unsigned long transforms_total;

    int id = -1;

    GLsync fence_sync = 0;

    bool bind_draw_buffer = true;
    BufferStorage() {
        static int count = 0;
        id = count++;
        transforms_total = 0;
    };
	~BufferStorage() {};
    void Reset() {
        fence_sync = 0;
        vertex_start = 0;
        index_total = 0;
        transforms_total = 0;
    };
    BufferMargins RequestStorage(float buffer_part_percent_vertex, float buffer_part_percent_index) {
        if (buffer_part_percent_vertex + allocated_percent_vertex > 1.0) {
            LOG((ostringstream() << "BufferStorage::RequestStorage failed for vertex. requested '" 
                << buffer_part_percent_vertex
                << "' + allocated '" 
                << allocated_percent_vertex
                << "' > 1").str());
            return {};
        }
        if (buffer_part_percent_index + allocated_percent_index > 1.0) {
            LOG((ostringstream() << "BufferStorage::RequestStorage failed for index. requested '"
                << buffer_part_percent_index
                << "' + allocated '"
                << allocated_percent_index
                << "' > 1").str());
            return {};
        }
        auto start_vertex = allocated_percent_vertex * MAX_VERTEX;
        auto start_index = allocated_percent_index * MAX_INDEX;
        auto end_vertex = (allocated_percent_vertex + buffer_part_percent_vertex) * MAX_VERTEX;
        auto end_index = (allocated_percent_index + buffer_part_percent_index) * MAX_INDEX;

        BufferMargins margins = { start_vertex, end_vertex, start_index, end_index };
        allocated_percent_vertex += buffer_part_percent_vertex;
        allocated_percent_index += buffer_part_percent_index;
        return margins;
    }
    bool operator<(const BufferStorage& other)
    {
        return id < other.id;
    }
    bool operator==(const BufferStorage& other)
    {
        return id == other.id;
    }

    // map buffers that updated asyncronously
    void MapBuffers() {
        for (auto buf : command_buffers) {
            lock_guard<mutex> data_lock(buf->buffer_lock->data_mutex);
            MapBuffer(buf);
        }
    }
    void MapBuffer(CommandBuffer* buf) {
        if (buf->buffer_lock->is_mapped == true) {
            return;
        }
        // MUST unmap INDIRECT DRAW pointer after buffering. Hence - map on demand.
        buf->ptr = (DrawElementsIndirectCommand*)glMapNamedBufferRange(buf->id, 0, buf->size * sizeof(DrawElementsIndirectCommand), flags);
        mapped_command_buffers[buf->id] = buf;
        buf->buffer_lock->is_mapped = true;
        buf->buffer_lock->is_mapped_cv.notify_all();
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
        for (auto& buf : mapped_command_buffers) {
            lock_guard<mutex> data_lock(buf.second->buffer_lock->data_mutex);
            UnmapBuffer(buf.second);
        }
        mapped_command_buffers.clear();
    }
    void UnmapBuffer(CommandBuffer* buf) {
        if (buf->buffer_lock->is_mapped == false) {
            return;
        }
        // MUST unmap GL_DRAW_INDIRECT_BUFFER. GL_INVALID_OPERATION otherwise.
        if (!glUnmapNamedBuffer(buf->id)) {
            CheckGLError();
        }
        buf->buffer_lock->is_mapped = false;
        buf->buffer_lock->is_mapped_cv.notify_all();
    }
    CommandBuffer* CreateCommandBuffer(unsigned int size) {
        auto buf = new CommandBuffer();
        buf->size = size;
        buf->buffer_lock = new BufferLock();
        buf->buffer_lock->is_mapped = false;
        
        // MUST unmap INDIRECT DRAW pointer after buffering. Hence - map on demand.
        glGenBuffers(1, &buf->id);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buf->id);
        glBufferStorage(GL_DRAW_INDIRECT_BUFFER, size * sizeof(DrawElementsIndirectCommand), NULL, flags);
        CheckGLError();
        return buf;
    }
    void ActivateCommandBuffer(CommandBuffer* buf) {
        lock_guard<mutex> data_lock(buf->buffer_lock->data_mutex);
        command_buffers.push_back(buf);
        MapBuffer(buf);
    }
    void RemoveCommandBuffer(CommandBuffer* to_remove) {
        lock_guard<mutex> data_lock(to_remove->buffer_lock->data_mutex);
        auto cur_buf = command_buffers.begin();
        while (cur_buf != command_buffers.end()) {
            if (*cur_buf == to_remove) {
                command_buffers.erase(cur_buf);
                mapped_command_buffers.erase(to_remove->id);
                UnmapBuffer(to_remove);
                break;
            }
            cur_buf++;
        }
    }
    void DeleteCommandBuffer(CommandBuffer* to_delete) {
        lock_guard<mutex> data_lock(to_delete->buffer_lock->data_mutex);
        auto cur_buf = command_buffers.begin();
        while (cur_buf != command_buffers.end()) {
            if (*cur_buf == to_delete) {
                command_buffers.erase(cur_buf);
                mapped_command_buffers.erase(to_delete->id);
                UnmapBuffer(to_delete);
                glDeleteBuffers(1, &(to_delete->id));
                break;
            }
            cur_buf++;
        }
        delete to_delete->buffer_lock;
        delete to_delete;
    }
    int AddCommands(vector<DrawElementsIndirectCommand>& active_commands, CommandBuffer* buf, int command_offset = -1) {
        unique_lock<mutex> data_lock(buf->buffer_lock->data_mutex);
        if (!buf->buffer_lock->is_mapped) {
            buf->buffer_lock->Wait(data_lock);
        }
        auto mapped_buffer = mapped_command_buffers.find(buf->id);
        int command_start = command_offset == -1 ? 0 : command_offset;
        if (mapped_buffer != mapped_command_buffers.end()) {
            {
                memcpy(&(mapped_buffer->second->ptr[command_start]), active_commands.data(), active_commands.size() * sizeof(DrawElementsIndirectCommand));
            }
            return command_start;
        }
        return -1;
    }
    int AddCommand(DrawElementsIndirectCommand& command, CommandBuffer* buf, int command_offset = -1) {
        unique_lock<mutex> data_lock(buf->buffer_lock->data_mutex);
        if (!buf->buffer_lock->is_mapped) {
            buf->buffer_lock->Wait(data_lock);
        }
        auto mapped_buffer = mapped_command_buffers.find(buf->id);
        if (mapped_buffer != mapped_command_buffers.end()) {
            int command_start = command_offset == -1 ? 0 : command_offset;
            buf->ptr[command_start] = command;
            return command_start;
        }
        return -1;
    }
    void BufferBoneTransform(map<unsigned int, mat4>& id_to_transform) {
        for (auto& id_trans : id_to_transform)
        {
            uniforms_ptr->bones_transforms[id_trans.first] = id_trans.second;
        }
        is_need_barrier = true;
    }
    void BufferBoneTransform(Bone* bone, mat4& trans, unsigned int num_bones = 1) {
        for (size_t i = 0; i < num_bones; i++)
        {
          uniforms_ptr->bones_transforms[bone[i].id] = trans;
        }
        is_need_barrier = true;
    }
    void BufferTransform(MeshData* mesh) {
        uniforms_ptr->transforms[mesh->transform_offset + mesh->instance_id] = mesh->transform;
        if (mesh->instance_id == 0) {
            uniforms_ptr->transform_offset[mesh->buffer_id + mesh->instance_id] = mesh->transform_offset;
        }
        is_need_barrier = true;
    }
    void BufferTransform(vector<MeshData*>& mesh) {
        for (int i = 0; i < mesh.size(); i++) {
            BufferTransform(mesh[i]);
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
            glDeleteBuffers(1, &(command_buffer->id));
            delete command_buffer;
        }
        command_buffers.clear();
        CheckGLError();
    }
};
