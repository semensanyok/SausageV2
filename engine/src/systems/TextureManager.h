#pragma once

#include "sausage.h"
#include "Structures.h"
#include "Texture.h"

class RawTextureData {
public:
  SDL_Surface* surface;
  RawTextureData(SDL_Surface* surface) : surface{surface} {};
  ~RawTextureData(){
    SDL_FreeSurface(surface);
  };
};

class TextureManager : public SausageSystem {
    map<size_t, Texture*> path_to_tex;
    Samplers* samplers;
public:
    TextureManager(Samplers* samplers) : samplers{ samplers } {};
    /**
    * load texture array for mesh. diffuse + normal + height + specular.
    */
    Texture* LoadTextureArray(MaterialTexNames& tex_names);

    unique_ptr<RawTextureData> LoadRawTextureData(string& path);
private:
    GLenum GetTexFormat(int bytes_per_pixel, bool for_storage);

    bool LoadLayer(string name, TextureType type);
};
