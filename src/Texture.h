#pragma once

#include "AssetUtils.h"
#include <string>
#include "SDL_image.h"
#include "Logging.h"

using namespace std;

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

Texture* LoadTexture(const char* path, const char* name, TextureType texture_type) {
    SDL_Surface* res_texture = IMG_Load(path);
    if (res_texture == NULL) {
        cerr << "IMG_Load: " << SDL_GetError() << endl;
        return nullptr;
    }
    Texture* texture = new Texture();
    texture->type = texture_type;
    texture->name = name;
    glGenTextures(1, &(texture->id));

    GLenum format;
    if (res_texture->format->BytesPerPixel == 3)
        format = GL_RGB;
    else if (res_texture->format->BytesPerPixel == 4)
        format = GL_RGBA;


    glBindTexture(GL_TEXTURE_2D, texture->id);

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

Samplers* InitSamplers() {
    Samplers* samplers = new Samplers();
    unsigned int basic_repeat = 0;
    glCreateSamplers(1, &basic_repeat);
    glSamplerParameteri(basic_repeat, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(basic_repeat, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glSamplerParameteri(basic_repeat, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(basic_repeat, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    //glSamplerParameteri(basic_repeat, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST); bilinear
    //glSamplerParameteri(basic_repeat, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR); trilinear
    //glSamplerParameteri(basic_repeat, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    samplers->basic_repeat = basic_repeat;

    return samplers;
}

void BindTexture(Texture* texture, unsigned int program) {
    // set name for texture
    glActiveTexture(GL_TEXTURE0 + (int)texture->type); // active proper texture unit before binding
    // now set the sampler to the correct texture unit
    int loc = glGetUniformLocation(program, texture->name);
    glUniform1i(loc, (int)texture->type);
    // and finally bind the texture
    glBindTexture(GL_TEXTURE_2D, texture->id);
}
