#pragma once

#include "sausage.h"
#include "Interfaces.h"
#include "BufferSettings.h"

using namespace std;

struct MaterialTexNames {
  const string diffuse;
  const string normal;
  const string specular;
  const string height;
  const string metal;
  const string ao;
  const string opacity;
};
/*
* diffuse considered as index
* other textures are auxiliary
*
* add other textures to hash/container funcions when needed
*/
// hash function for unordered map
template<> struct std::hash<MaterialTexNames> {
  size_t operator()(MaterialTexNames const& t) const {
    return hash<string>{}(string(t.diffuse));
  }
};
// eq for hashmap/hashset
inline bool operator==(const MaterialTexNames& lhs, const MaterialTexNames& rhs) {
  return lhs.diffuse == rhs.diffuse;
}
// compare for map/set
inline bool operator<(const MaterialTexNames& lhs, const MaterialTexNames& rhs) {
  return lhs.diffuse < rhs.diffuse;
}

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


// largest base alignment value of any of its members == 4 bytes
struct TextureBlend {
  float blend_weight; // 4 bytes
  // monotonically increasing, Sausage managed. see Texture->id.
  unsigned int texture_id; // 4 bytes for 64 bit build
  // no padding needed, as all members are of equal size
};

// largest base alignment value of any of its members == 4 bytes
struct BlendTextures {
  TextureBlend textures[BufferSettings::MAX_BLEND_TEXTURES]; // alignment 4 bytes
  unsigned int num_textures; // alignment 4 bytes for 64 bit build
  // no padding needed, as all members are of equal size
};
