#pragma once

#include "sausage.h"
#include "utils/AssetUtils.h"
#include "SDL_image.h"
#include "Logging.h"
#include "OpenGLHelpers.h"
#include "structures/TextureStruct.h"

using namespace std;

/**
* Stores id and handle for texture array.
*/
class Texture {
    bool is_destoyed;
    bool is_resident;
public:
    const SausageHashable* hash;

    // monotonically increasing, Sausage managed
    const unsigned int id;
    // GL managed
    const GLuint texture_id;
    // GL managed
    const GLuint64 texture_handle_ARB;
    Texture(GLuint texture_id, GLuint64 texture_handle_ARB, SausageHashable* hash, unsigned int id)
      : texture_id(texture_id), texture_handle_ARB(texture_handle_ARB),
      hash{ hash }, is_resident(false), is_destoyed(false), id{ id } {};
    void MakeResident();
    void MakeNonResident();
    void BindSingleSampler(unsigned int location);
    void Dispose();
    ~Texture();
};
