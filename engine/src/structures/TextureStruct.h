#pragma once

#include "sausage.h"
#include "Interfaces.h"
#include "BufferSettings.h"

using namespace std;

struct MaterialTexNames {
  string diffuse;
  string normal;
  string specular;
  string height;
  string metal;
  string ao;
  string opacity;
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
  // KEEP SORTED BY TEXTURE ID! (to deduplicate)
  TextureBlend textures[BufferSettings::MAX_BLEND_TEXTURES]; // alignment 4 bytes
  unsigned int num_textures; // alignment 4 bytes for 64 bit build
  // no padding needed, as all members are of equal size
};
// hash function for unordered map
template<> struct std::hash<BlendTextures> {
  size_t operator()(BlendTextures const& t) const {
    auto s1 = ostringstream() << t.num_textures;
    for (int i = 0; i < min(t.num_textures, BufferSettings::MAX_BLEND_TEXTURES); i++) {
      auto tex = t.textures[i];
      s1 << tex.texture_id << tex.blend_weight;
    }
    return hash<string>{}(s1.str());
  }
};
// eq for hashmap/hashset
inline bool operator==(const BlendTextures& lhs, const BlendTextures& rhs) {
  if (lhs.num_textures != rhs.num_textures) {
    return false;
  }
  for (int i = 0; i < lhs.num_textures; i++) {
    auto t1 = lhs.textures[i];
    auto t2 = rhs.textures[i];
    if (lhs.textures[i].texture_id != rhs.textures[i].texture_id) {
      return false;
    }
    float eps = 0.01; // 1%
    if (fabs(lhs.textures[i].blend_weight - rhs.textures[i].blend_weight) >= eps) {
      return false;
    }

  }
  return true;
}
// compare for map/set
inline bool operator<(const BlendTextures& lhs, const BlendTextures& rhs) {
  if (lhs.num_textures != rhs.num_textures) {
    return lhs.num_textures < rhs.num_textures;
  }
  for (int i = 0; i < lhs.num_textures; i++) {
    if (lhs.textures[i].texture_id != rhs.textures[i].texture_id) {
      return lhs.textures[i].texture_id < rhs.textures[i].texture_id;
    }
    float eps = 0.01; // 1%
    if (fabs(lhs.textures[i].blend_weight - rhs.textures[i].blend_weight) >= eps) {
      return lhs.textures[i].blend_weight < rhs.textures[i].blend_weight;
    }
  }
  return false;
}
