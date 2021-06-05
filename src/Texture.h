#pragma once

#include "sausage.h"
#include "utils/AssetUtils.h"
#include "SDL_image.h"
#include "Logging.h"
#include "Structures.h"
#include "OpenGLHelpers.h"

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

GLenum GetTexFormat(int bytes_per_pixel, bool for_storage) {
    if (bytes_per_pixel == 3) {
        return for_storage ? GL_RGB8 : GL_RGB;
    }
    else if (bytes_per_pixel == 4) {
        return for_storage ? GL_RGBA8 : GL_RGBA;
    }
    LOG((ostringstream() << "Unknown texture format. surface->format->BytesPerPixel == " << bytes_per_pixel).str());
    return for_storage ? GL_RGBA8 : GL_RGBA;
}

bool LoadLayer(string name, TextureType type) {
    SDL_Surface* surface = IMG_Load(GetTexturePath(name).c_str());
    if (surface == NULL)
    {
        LOG((ostringstream() << "IMG_Load: " << SDL_GetError()).str());
        return false;
    }
    else
    {
        CheckGLError();
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
            0,
            0, 0, (int)type,
            surface->w, surface->h, 1,
            GetTexFormat(surface->format->BytesPerPixel, false),
            GL_UNSIGNED_BYTE,
            surface->pixels);
        CheckGLError();
        SDL_FreeSurface(surface);
        return true;
    }
}

/**
* load texture array for mesh. diffuse + normal + height + specular.
*/
Texture* LoadTextureArray(GLuint texture_sampler,
    MaterialTexNames& tex_names) {
    GLuint texture_id;
    if (tex_names.diffuse_name.empty()) {
        LOG(string("diffuse not found: ").append(tex_names.diffuse_name));
        return nullptr;
    }
    // diffuse is a must. used to create storage. all textures must be same size and format.
    SDL_Surface* surface = IMG_Load(GetTexturePath(tex_names.diffuse_name).c_str());
    glGenTextures(1, &texture_id);
    CheckGLError();
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id);
    CheckGLError();
    glTexStorage3D(GL_TEXTURE_2D_ARRAY,
        1,                    //mipmaps. 1 == no mipmaps.
        GetTexFormat(surface->format->BytesPerPixel, true),              //Internal format
        surface->w, surface->h,//width,height
        4            //Number of layers
    );
    CheckGLError();
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
        0,                     //Mipmap number
        0, 0, (int)TextureType::Diffuse,               //xoffset, yoffset, zoffset
        surface->w, surface->h, 1,               //width, height, depth
        GetTexFormat(surface->format->BytesPerPixel, false),                //format
        GL_UNSIGNED_BYTE,      //type
        surface->pixels);
    CheckGLError();
    SDL_FreeSurface(surface);
    if (!tex_names.normal_name.empty()) {
        LoadLayer(tex_names.normal_name, TextureType::Normal);
    }
    if (!tex_names.specular_name.empty()) {
        LoadLayer(tex_names.specular_name, TextureType::Specular);
    }
    if (!tex_names.height_name.empty()) {
        LoadLayer(tex_names.height_name, TextureType::Height);
    }
    // Invalid operation GL error.
    //if (!tex_names.metal_name.empty()) {
    //    LoadLayer(tex_names.metal_name, TextureType::Metal);
    //}
    if (!tex_names.ao_name.empty()) {
        LoadLayer(tex_names.ao_name, TextureType::AO);
    }
    if (!tex_names.opacity_name.empty()) {
        LoadLayer(tex_names.opacity_name, TextureType::Opacity);
    }
    GLuint64 tex_handle = glGetTextureSamplerHandleARB(texture_id, texture_sampler);
    CheckGLError();
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
