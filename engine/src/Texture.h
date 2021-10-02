#pragma once

#include "sausage.h"
#include "utils/AssetUtils.h"
#include "SDL_image.h"
#include "Logging.h"
#include "Structures.h"
#include "OpenGLHelpers.h"

using namespace std;

/**
* Stores id and handle for texture array.
*/
class Texture {
    bool is_destoyed;
    bool is_resident;
    unsigned int ref_count;
public:
    const MaterialTexNames tex_names;

    const GLuint texture_id;
    const GLuint64 texture_handle_ARB;
    Texture(GLuint texture_id, GLuint64 texture_handle_ARB, MaterialTexNames tex_names) : texture_id(texture_id), texture_handle_ARB(texture_handle_ARB), 
        tex_names{tex_names}, is_resident(false), is_destoyed(false), ref_count(0) {};
    void MakeResident();
    void MakeNonResident();
    void BindSingleSampler(unsigned int location);
    void Dispose();
    ~Texture();
};
