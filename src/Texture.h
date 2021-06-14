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
    bool is_destoyed = false;
    bool is_resident = false;
    unsigned int ref_count;
public:
    const string diffuse_path;
    const string normal_path;
    const string specular_path;
    const string height_path;

    const GLuint texture_id;
    const GLuint64 texture_handle_ARB;
    Texture(GLuint texture_id, GLuint64 texture_handle_ARB) : texture_id(texture_id), texture_handle_ARB(texture_handle_ARB), is_resident(false), is_destoyed(false) {};
    void MakeResident() {
        if (is_destoyed) {
            LOG("ERROR making texutre resident. texture is destroyed");
            return;
        }
        if (!is_resident) {
            glMakeTextureHandleResidentARB(texture_handle_ARB);
            is_resident = true;
        }
    }
    void MakeNonResident() {
        if (is_resident) {
            glMakeTextureHandleNonResidentARB(texture_handle_ARB);
            is_resident = false;
        }
    }
    void BindSingleSampler(unsigned int location) {
        glUniformHandleui64ARB(location, texture_handle_ARB);
    }
    void Dispose() {
        MakeNonResident();
        glDeleteTextures(1, &texture_id);
        is_destoyed = true;
    }
    ~Texture() {
        if (!is_destoyed) {
            Dispose();
        };
    }
};

Samplers InitSamplers() {
    Samplers samplers = Samplers();
    unsigned int basic_repeat = 0;
    glCreateSamplers(1, &basic_repeat);
    glSamplerParameteri(basic_repeat, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(basic_repeat, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glSamplerParameteri(basic_repeat, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(basic_repeat, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    //glSamplerParameteri(basic_repeat, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST); bilinear
    //glSamplerParameteri(basic_repeat, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR); trilinear
    //glSamplerParameteri(basic_repeat, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    samplers.basic_repeat = basic_repeat;

    return samplers;
}
