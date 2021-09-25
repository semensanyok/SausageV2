#pragma once

#include "sausage.h"
#include "Structures.h"
#include "Texture.h"

class TextureManager {
    map<size_t, Texture*> path_to_tex;
public:
    Samplers samplers;
    bool is_samplers_init = false;
    inline TextureManager() {};
    /**
    * load texture array for mesh. diffuse + normal + height + specular.
    */
    Texture* LoadTextureArray(MaterialTexNames& tex_names);

    void InitSamplers();

private:
    GLenum GetTexFormat(int bytes_per_pixel, bool for_storage);

    bool LoadLayer(string name, TextureType type);
};