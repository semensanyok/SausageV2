#pragma once

#include "sausage.h"
#include "Interfaces.h"

using namespace std;

struct MaterialTexNames : SausageHashable {
public:
  string diffuse;
  string normal;
  string specular;
  string height;
  string metal;
  string ao;
  string opacity;
  MaterialTexNames(string diffuse,
  string normal,
  string specular,
  string height,
  string metal,
  string ao,
  string opacity) :
    diffuse{ diffuse },
    normal{ normal },
    specular{ specular },
    height{ height },
    metal{ metal },
    ao{ ao } {}
  size_t Hash() {
    return hash<string>{}(string(diffuse)
                              .append(normal)
                              .append(specular)
                              .append(height)
                              .append(metal)
                              .append(ao)
                              .append(opacity));
  }
};

enum TextureType {
  Diffuse,   // sampler2DArray   index 0
  Normal,    //                        1
  Specular,  //                        2 roughness in PBR
  AO,        //                        3
  Height,    //                        4 PBR
  Metal,     //                        5 PBR
             //                        6 Opacity, (switched to use Diffuse alpha)
};


class RawTextureData {
public:
  SDL_Surface* surface;
  RawTextureData(SDL_Surface* surface) : surface{ surface } {};
  ~RawTextureData() {
    SDL_FreeSurface(surface);
  };
};
