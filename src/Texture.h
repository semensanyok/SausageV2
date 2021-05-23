#pragma once

#include "sausage.h"
#include "systems/BufferStorage.h"
#include "AssetUtils.h"
#include "SDL_image.h"
#include "Logging.h"
#include "Structures.h"

using namespace std;

class Texture {
    bool is_destoyed = false;
    bool is_resident = false;
    unsigned int ref_count;
public:
    const GLuint texture_id;
    const GLuint64 texture_handle_ARB;
    Texture(GLuint texture_id, GLuint64 texture_handle_ARB) : texture_id(texture_id), texture_handle_ARB(texture_handle_ARB), is_resident(false), is_destoyed(false) {};
    void MakeResident() {
        if (is_destoyed) {
            LOG("ERROR making texutre resident. texture is destroyed");
            return;
        }
        if (!is_resident) {
            glMakeTextureHandleResidentARB(texture_handle_ARB);
            is_resident = true;
        }
    }
    void MakeNonResident() {
        if (is_resident) {
            glMakeTextureHandleNonResidentARB(texture_handle_ARB);
            is_resident = false;
        }
    }
    void BindSingleSampler(unsigned int location) {
        glUniformHandleui64ARB(location, texture_handle_ARB);
    }
    void Dispose() {
        MakeNonResident();
        glDeleteTextures(1, &texture_id);
        is_destoyed = true;
    }
    ~Texture() {
        if (!is_destoyed) {
            Dispose();
        };
    }
};
string GetTexturePath(const string& name, TextureType texture_type) {
    switch (texture_type)
    {
    case TextureType::Diffuse:
        return name;
        break;
    case TextureType::Specular:
        return name + "_specular";
        break;
    case TextureType::Normal:
        return name + "_normal";
        break;
    case TextureType::Height:
        return name + "_height";
        break;
    default:
        return name;
        break;
    }
}

bool LoadLayer(const char* name, TextureType type) {
    SDL_Surface* surface = IMG_Load(GetTexturePath(name).c_str());
    if (surface == NULL)
    {
        LOG((ostringstream() << "IMG_Load: " << SDL_GetError()).str());
        return false;
    }
    else
    {
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
            0,
            0, 0, (int)type,
            surface->w, surface->h, 1,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            surface->pixels);
        SDL_FreeSurface(surface);
        return true;
    }
}

/**
* load texture array for mesh. diffuse + normal + height + specular.
*/
Texture* LoadTextureArray(const char* diffuse_name, const char* normal_name, const char* specular_name, const char* height_name, BufferStorage* buffer_storage) {
    GLuint texture_id;
    if (diffuse_name == nullptr) {
        LOG(string("diffuse not found: ").append(diffuse_name));
        return nullptr;
    }
    
    // diffuse is a must. used to create storage. all textures must be same size and format.
    SDL_Surface* surface = IMG_Load(GetTexturePath(diffuse_name).c_str());
    if (surface == NULL) {
        cerr << "IMG_Load: " << SDL_GetError() << endl;
        return nullptr;
    }
    GLenum format;
    if (surface->format->BytesPerPixel == 3)
        format = GL_RGB;
    else if (surface->format->BytesPerPixel == 4)
        format = GL_RGBA;
    else {
        LOG((ostringstream() << "Unknown texture format. surface->format->BytesPerPixel == " << surface->format->BytesPerPixel).str());
    }
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY,
        1,                    //mipmaps. 1 == no mipmaps.
        format,              //Internal format
        surface->w, surface->h,//width,height
        4            //Number of layers
    );
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
        0,                     //Mipmap number
        0, 0, (int)TextureType::Diffuse,               //xoffset, yoffset, zoffset
        surface->w, surface->h, 1,               //width, height, depth
        format,                //format
        GL_UNSIGNED_BYTE,      //type
        surface->pixels);
    SDL_FreeSurface(surface);

    // normal
    if (normal_name != nullptr) {
        LoadLayer(normal_name, TextureType::Normal);
    }

    // specular
    if (specular_name != nullptr) {
        LoadLayer(specular_name, TextureType::Specular);
    }

    // height
    if (height_name != nullptr) {
        LoadLayer(height_name, TextureType::Height);
    }
    GLuint64 tex_handle = buffer_storage->AllocateTextureHandle(texture_id);
    Texture* texture = new Texture(texture_id, tex_handle);
    return texture;
}

Samplers InitSamplers() {
    Samplers samplers = Samplers();
    unsigned int basic_repeat = 0;
    glCreateSamplers(1, &basic_repeat);
    glSamplerParameteri(basic_repeat, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(basic_repeat, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glSamplerParameteri(basic_repeat, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(basic_repeat, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    //glSamplerParameteri(basic_repeat, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST); bilinear
    //glSamplerParameteri(basic_repeat, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR); trilinear
    //glSamplerParameteri(basic_repeat, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    samplers.basic_repeat = basic_repeat;

    return samplers;
}
