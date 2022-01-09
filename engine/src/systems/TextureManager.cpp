#include "TextureManager.h"

/**
* load texture array for mesh. diffuse + normal + height + specular.
*/

Texture* TextureManager::LoadTextureArray(MaterialTexNames& tex_names) {
    GLuint texture_id;
    if (tex_names.diffuse.empty()) {
        LOG(string("diffuse not found: ").append(tex_names.diffuse));
        return nullptr;
    }
    // diffuse is a must. used to create storage. all textures must be same size and format.
    string diffuse_path = GetTexturePath(tex_names.diffuse);
    string normal_path = GetTexturePath(tex_names.normal);
    string specular_path = GetTexturePath(tex_names.specular);
    string height_path = GetTexturePath(tex_names.height);
    auto key_hash = tex_names.Hash();

    auto existing = path_to_tex.find(key_hash);
    if (existing != path_to_tex.end()) {
        LOG((ostringstream() << "Texture array already loaded: " <<
            diffuse_path + ", " + normal_path + ", " + specular_path + ", " + height_path).str());
        return (*existing).second;
    }
    SDL_Surface* surface = IMG_Load(diffuse_path.c_str());
    if (surface == NULL)
    {
        LOG((ostringstream() << "Error on IMG_Load: " << SDL_GetError()).str());
        return nullptr;
    }
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY,
        8,                    //mipmaps. 1 == no mipmaps.
        GetTexFormat(surface->format->BytesPerPixel, true),              //Internal format
        surface->w, surface->h,//width,height
        4            //Number of layers
    );
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
        0,                     //Mipmap number
        0, 0, (int)TextureType::Diffuse,               //xoffset, yoffset, zoffset
        surface->w, surface->h, 1,               //width, height, depth
        GetTexFormat(surface->format->BytesPerPixel, false),                //format
        GL_UNSIGNED_BYTE,      //type
        surface->pixels);

    SDL_FreeSurface(surface);
    if (!tex_names.normal.empty()) {
        LoadLayer(tex_names.normal, TextureType::Normal);
    }
    if (!tex_names.specular.empty()) {
        LoadLayer(tex_names.specular, TextureType::Specular);
    }
    if (!tex_names.height.empty()) {
        LoadLayer(tex_names.height, TextureType::Height);
    }
    // Invalid operation GL error.
    //if (!tex_names.metal_name.empty()) {
    //    LoadLayer(tex_names.metal_name, TextureType::Metal);
    //}
    if (!tex_names.ao.empty()) {
        LoadLayer(tex_names.ao, TextureType::AO);
    }
    //if (!tex_names.opacity.empty()) {
    //    LoadLayer(tex_names.opacity, TextureType::Opacity);
    //}
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    GLuint64 tex_handle = glGetTextureSamplerHandleARB(texture_id, samplers->basic_repeat);
    CheckGLError();
    Texture* texture = new Texture(texture_id, tex_handle, tex_names);
    path_to_tex[key_hash] = texture;
    return texture;
}

unique_ptr<RawTextureData> TextureManager::LoadRawTextureData(string& path)
{
  SDL_Surface* surface = IMG_Load(path.c_str());
  return make_unique<RawTextureData>(surface);
}

GLenum TextureManager::GetTexFormat(int bytes_per_pixel, bool for_storage) {
    if (bytes_per_pixel == 3) {
        return for_storage ? GL_RGB8 : GL_RGB;
    }
    else if (bytes_per_pixel == 4) {
        return for_storage ? GL_RGBA8 : GL_RGBA;
    }
    LOG((ostringstream() << "Unknown texture format. surface->format->BytesPerPixel == " << bytes_per_pixel).str());
    return for_storage ? GL_RGBA8 : GL_RGBA;
}

bool TextureManager::LoadLayer(string name, TextureType type) {
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
