#pragma once

#include "sausage.h"
#include "AssetUtils.h"
#include "SDL_image.h"
#include "OpenGLHelpers.h"
#include "TextureStruct.h"
#include "Macros.h"
#include "Logging.h"

using namespace std;

/**
* Stores id and handle for texture array.
*/
class Texture {
  //friend class TextureManager;
  friend class BufferStorage;
    bool is_destoyed;
    bool is_resident;
public:
    size_t* material_tex_names_hash;
    // monotonically increasing, Sausage managed
    const unsigned int id;
    // GL managed
    const GLuint texture_id;
    // GL managed
    const GLuint64 texture_handle_ARB;
    void MakeResident();
    void MakeNonResident();
    void BindSingleSampler(unsigned int location);
private:
    Texture(GLuint texture_id, GLuint64 texture_handle_ARB, unsigned int id)
      : texture_id(texture_id), texture_handle_ARB(texture_handle_ARB),
      is_resident(false), is_destoyed(false), id{ id }, material_tex_names_hash{ nullptr } {};
    void Dispose();
    ~Texture();
};

// hash function for unordered map
template<> struct std::hash<Texture> {
  size_t operator()(Texture const& t) const {
    return t.id;
  }
};

// eq for hashmap/hashset
inline bool operator==(const Texture& lhs, const Texture& rhs) {
  return lhs.id == rhs.id;
}
// compare for map/set
inline bool operator<(const Texture& lhs, const Texture& rhs) {
  return lhs.id < rhs.id;
}
