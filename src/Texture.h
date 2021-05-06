#pragma once

#include <string>
#include "SDL_image.h"

using namespace std;

enum class TextureType
{
    Diffuse,
    Specular,
    Normal,
    Height,
};

struct Texture {
    unsigned int id;
    TextureType type;
};

string GetTextureName(Texture& texture, int texture_num) {
    switch (texture.type)
    {
    case TextureType::Diffuse:
        return "texture_diffuse" + to_string(texture_num);
        break;
    case TextureType::Specular:
        "texture_specular" + to_string(texture_num);
        break;
    case TextureType::Normal:
        "texture_normal" + to_string(texture_num);
        break;
    case TextureType::Height:
        "texture_height" + to_string(texture_num);
        break;
    default:
        break;
    }
    return {};
}

Texture* LoadTexture(const char* path, TextureType texture_type) {
    SDL_Surface* res_texture = IMG_Load(path);
    if (res_texture == NULL) {
        cerr << "IMG_Load: " << SDL_GetError() << endl;
        return nullptr;
    }
    Texture* texture = new Texture();
    texture->type = texture_type;
    glGenTextures(1, &(texture->id));

    //GLenum format;
    //if (nrComponents == 1)
    //    format = GL_RED;
    //else if (nrComponents == 3)
    //    format = GL_RGB;
    //else if (nrComponents == 4)
    //    format = GL_RGBA;

    GLenum format;
    if (res_texture->format->BytesPerPixel == 3)
        format = GL_RGB;
    else if (res_texture->format->BytesPerPixel == 4)
        format = GL_RGBA;


    glBindTexture(GL_TEXTURE_2D, texture->id);

    //glGenerateMipmap(GL_TEXTURE_2D);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, // target
        0, // level, 0 = base, no minimap,
        GL_RGBA, // internalformat
        res_texture->w, // width
        res_texture->h, // height
        0, // border, always 0 in OpenGL ES
        GL_RGBA, // format
        GL_UNSIGNED_BYTE, // type
        res_texture->pixels);
    SDL_FreeSurface(res_texture);
    return texture;
}