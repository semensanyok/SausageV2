#pragma once

#include "sausage.h"
#include "Texture.h"

class TextureManager {
    map<string, Texture*> path_to_tex;
public:
    TextureManager() {};
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
        string diffuse_path = GetTexturePath(tex_names.diffuse_name);
        string normal_path = GetTexturePath(tex_names.normal);
        string diffuse_path = GetTexturePath(tex_names.diffuse_name);
        string diffuse_path = GetTexturePath(tex_names.diffuse_name);
        string diffuse_path = GetTexturePath(tex_names.diffuse_name);

        auto existing = _RegisterOrGetIfExists(path);

        if (existing != nullptr) {
            return existing;
        }
        SDL_Surface* surface = IMG_Load(path.c_str());
        {
            LOG((ostringstream() << "Error on IMG_Load: " << SDL_GetError()).str());
            return false;
        }

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
private:
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
            LOG((ostringstream() << "Error on IMG_Load: " << SDL_GetError()).str());
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

    Texture* _RegisterOrGetIfExists(string& path) {
        auto existing = path_to_tex.find(path)
        if (existing != path_to_tex.end()) {
            LOG((ostringstream() << "Texture " << path << " already loaded. Returning existing tex array.").str())
            return *existing;
        }
        return nullptr;
    }
};