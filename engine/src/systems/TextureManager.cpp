#include "TextureManager.h"

/**
* load texture array for mesh. diffuse + normal + height + specular.
*/

Texture* TextureManager::LoadTextureArray(MaterialTexNames& tex_names) {
    if (tex_names.diffuse.empty()) {
        LOG(string("diffuse not found: ").append(tex_names.diffuse));
        return nullptr;
    }
    // diffuse is a must. used to create storage. all textures must be same size and format.
    string diffuse_path = GetTexturePath(tex_names.diffuse);
    string normal_path = GetTexturePath(tex_names.normal);
    string specular_path = GetTexturePath(tex_names.specular);
    string height_path = GetTexturePath(tex_names.height);

    hash<MaterialTexNames> mtn_hash;
    auto existing = texture_by_material_tex_names_hash.find(mtn_hash(tex_names));
    if (existing != texture_by_material_tex_names_hash.end()) {
        LOG((ostringstream() << "Texture array already loaded: " <<
            diffuse_path + ", " + normal_path + ", " + specular_path + ", " + height_path).str());
        texture_used_count_by_id[existing->second->id]++;
        return existing->second;
    }
    SDL_Surface* surface = IMG_Load(diffuse_path.c_str());
    if (surface == NULL)
    {
        LOG((ostringstream() << "Error on IMG_Load: " << SDL_GetError()).str());
        return nullptr;
    }
    GLuint texture_id = AllocateGLTextureId();

    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY,
        8,                    //mipmaps. 1 == no mipmaps.
        GetTexFormat(surface->format->BytesPerPixel, true), //Internal format
        surface->w, surface->h,//width,height
        4            //Number of layers
    );
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
        0,                               //Mipmap number
        0, 0, (int)TextureType::Diffuse, //xoffset, yoffset, zoffset
        surface->w, surface->h, 1,       //width, height, depth
        GetTexFormat(surface->format->BytesPerPixel, false), //format
        GL_UNSIGNED_BYTE,      //type
        surface->pixels);

    //int max_texture_array_levels;
    //glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &max_texture_array_levels);

    SDL_FreeSurface(surface);
    if (!tex_names.normal.empty()) {
        LoadLayer(tex_names.normal, TextureType::Normal);
    }
    if (!tex_names.specular.empty()) {
        LoadLayer(tex_names.specular, TextureType::Specular);
    }
    if (!tex_names.height.empty()) {
        LoadLayer(tex_names.height, TextureType::Height);
    }
    // Invalid operation GL error.
    //if (!tex_names.metal_name.empty()) {
    //    LoadLayer(tex_names.metal_name, TextureType::Metal);
    //}
    if (!tex_names.ao.empty()) {
        LoadLayer(tex_names.ao, TextureType::AO);
    }
    //if (!tex_names.opacity.empty()) {
    //    LoadLayer(tex_names.opacity, TextureType::Opacity);
    //}
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    DEBUG_EXPR(CheckGLError());

    Texture* texture = CreateTexture(texture_id, samplers->basic_repeat);

    texture->material_tex_names_hash = new size_t(mtn_hash(tex_names));
    texture_by_material_tex_names_hash[mtn_hash(tex_names)] = texture;
    texture_used_count_by_id[texture->id] = 1;

    return texture;
}


/**
* This function operates mostly like SDL_CreateRGBSurface(),
except it does not allocate memory for the pixel data,
instead the caller provides an existing buffer of data for the surface to use.
* No copy is made of the pixel data.
Pixel data is not managed automatically;
you must free the surface before you free the pixel data.
*
* @param pixels
* @param format
* @param width
* @param height
* @param depth number of bits per pixel. 24 for RGB.
* @param pitch
* @param Rmask e.g. 0xff0000 for RGB
* @param Gmask e.g. 0x00ff00 for RGB
* @param Bmask e.g. 0x0000ff for RGB
* @param Amask e.g. 0 for RGB
* @param num of mipmaps to generate
*        mipmap. max == take logarithm inverse because
*        GL_INVALID_OPERATION is generated if target is GL_TEXTURE_3D or GL_PROXY_TEXTURE_3D
*        and levels is greater than log2(max(width, height, depth))+1
* @param diffuse_tex_name name for hash, to deduplicate loaded by same name
* @param normal_tex_name name for hash, to deduplicate loaded by same name
* @param specular_tex_name name for hash, to deduplicate loaded by same name
* @return
*/

Texture* TextureManager::GenTextureArray(
  void* pixels_diffuse, void* pixels_normal, void* pixels_specular,
  int width, int height,
  int depth, int pitch,
  Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask,
  GLsizei levels,
  string diffuse_tex_name, string normal_tex_name, string specular_tex_name) {
  SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(pixels_diffuse, width, height, depth, pitch,
    Rmask, Gmask, Bmask, Amask);
  auto max_allowed_levels = log2(std::max((float)width, (float)height));
  if (levels > max_allowed_levels) {
    levels = max_allowed_levels;
  }
  assert(levels <= max_allowed_levels);
  if (surface == NULL)
  {
    LOG((ostringstream() << "Error on IMG_Load: " << SDL_GetError()).str());
    return nullptr;
  }
  GLuint texture_id = AllocateGLTextureId();
  DEBUG_EXPR(CheckGLError());
  glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id);
  glTexStorage3D(GL_TEXTURE_2D_ARRAY,
    levels,                    //mipmaps. 1 == no mipmaps.
    GetTexFormat(surface->format->BytesPerPixel, true), //Internal format
    surface->w, surface->h,//width,height
    3            //Number of layers
  );
  DEBUG_EXPR(CheckGLError());
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
    0,                               //Mipmap number
    0, 0, (int)TextureType::Diffuse, //xoffset, yoffset, zoffset
    surface->w, surface->h, 1,       //width, height, depth
    GetTexFormat(surface->format->BytesPerPixel, false), //format
    GL_UNSIGNED_BYTE,      //type
    surface->pixels);
  DEBUG_EXPR(CheckGLError());
  //int max_texture_array_levels;
  //glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &max_texture_array_levels);
  SDL_FreeSurface(surface);
  {
    surface = SDL_CreateRGBSurfaceFrom(pixels_normal, width, height, depth, pitch,
      Rmask, Gmask, Bmask, Amask);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
      0,
      0, 0, (int)TextureType::Normal,
      surface->w, surface->h, 1,
      GetTexFormat(surface->format->BytesPerPixel, false),
      GL_UNSIGNED_BYTE,
      surface->pixels);
    DEBUG_EXPR(CheckGLError());
    SDL_FreeSurface(surface);
  }
  {
    surface = SDL_CreateRGBSurfaceFrom(pixels_diffuse, width, height, depth, pitch,
      Rmask, Gmask, Bmask, Amask);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
      0,
      0, 0, (int)TextureType::Specular,
      surface->w, surface->h, 1,
      GetTexFormat(surface->format->BytesPerPixel, false),
      GL_UNSIGNED_BYTE,
      surface->pixels);
    DEBUG_EXPR(CheckGLError());
    SDL_FreeSurface(surface);
  }
  glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
  DEBUG_EXPR(CheckGLError());

  Texture* texture = CreateTexture(texture_id, samplers->basic_repeat);
  MaterialTexNames tex_names = { diffuse_tex_name, normal_tex_name, specular_tex_name };
  hash<MaterialTexNames> mtn_hash;
  texture->material_tex_names_hash = new size_t(mtn_hash(tex_names));
  texture_by_material_tex_names_hash[mtn_hash(tex_names)] = texture;
  texture_used_count_by_id[texture->id] = 1;

  return texture;
}

unique_ptr<RawTextureData> TextureManager::LoadRawTextureData(string& path)
{
  SDL_Surface* surface = IMG_Load(path.c_str());
  return make_unique<RawTextureData>(surface);
}

GLenum TextureManager::GetTexFormat(int bytes_per_pixel, bool for_storage) {
    if (bytes_per_pixel == 3) {
        return for_storage ? GL_RGB8 : GL_RGB;
    }
    else if (bytes_per_pixel == 4) {
        return for_storage ? GL_RGBA8 : GL_RGBA;
    }
    LOG((ostringstream() << "Unknown texture format. surface->format->BytesPerPixel == " << bytes_per_pixel).str());
    return for_storage ? GL_RGBA8 : GL_RGBA;
}

GLuint TextureManager::AllocateGLTextureId()
{
  GLuint texture_id;
  glGenTextures(1, &texture_id);
  DEBUG_EXPR(CheckGLError());
  return texture_id;
}

/**
  * https://www.khronos.org/opengl/wiki/Bindless_Texture
    Handle creation
    Texture handles are created using glGetTextureHandleARB or glGetTextureSamplerHandleARB.
    GLuint64 glGetTextureHandleARB(GLuint texture​);
    GLuint64 glGetTextureSamplerHandleARB(GLuint texture​, GLuint sampler​);
        These functions accept the name of a texture object and optionally a sampler object
      to produce a texture handle.
        Multiple invocations with the same texture (or texture/sampler pair) will produce the same handle.
        Once a handle is created for a texture/sampler, none of its state can be changed.
        For Buffer Textures, this includes the buffer object that is currently attached to it
      (which also means that you cannot create a handle for a buffer texture
      that does not have a buffer attached).
        Not only that, in such cases the buffer object itself becomes immutable;
      it cannot be reallocated with glBufferData.
        Though just as with textures, its storage can still be mapped and have its data modified
      by other functions as normal.

    * That means -
      call glGetTextureSamplerHandleARB
      AFTER buffering data.
      cannot call it beforehand
*/
Texture* TextureManager::CreateTexture(GLuint texture_id, GLuint sampler)
{
  GLuint64 tex_handle = glGetTextureSamplerHandleARB(texture_id, sampler);
  DEBUG_EXPR(CheckGLError());
  return BufferStorage::GetInstance()->CreateTextureWithBufferSlot(texture_id, tex_handle);
}

void TextureManager::Dispose(Texture* texture)
{
  texture_used_count_by_id[texture->id] = texture_used_count_by_id[texture->id] - 1;
  if (texture_used_count_by_id[texture->id] < 1) {
    if (texture->material_tex_names_hash != nullptr) {
      texture_by_material_tex_names_hash.erase(*(texture->material_tex_names_hash));
    }
    BufferStorage::GetInstance()->ReleaseTexture(texture);
  }
}

/**
 * @brief glBindTexture must be called beforehand with texture_id for which to buffer data
*/
bool TextureManager::LoadLayer(string name, TextureType type) {
  SDL_Surface* surface = IMG_Load(GetTexturePath(name).c_str());
  if (surface == NULL)
  {
    LOG((ostringstream() << "Error on IMG_Load: " << SDL_GetError()).str());
    return false;
  }
  else
  {
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
        0,
        0, 0, (int)type,
        surface->w, surface->h, 1,
        GetTexFormat(surface->format->BytesPerPixel, false),
        GL_UNSIGNED_BYTE,
        surface->pixels);
    DEBUG_EXPR(CheckGLError());
    SDL_FreeSurface(surface);
    return true;
  }
}
