#pragma once

#include "sausage.h"
#include "Structures.h"
#include "Interfaces.h"
#include "GPUStructs.h"
#include "Texture.h"
#include "BufferManager.h"
#include "Macros.h"
#include "ThreadSafeNumberPool.h"

/**

for simplicity implemented texture as "bindless 2D array",
not using "sparse" textures (which allows making resident only some layers of GL texture)
  - for mesh use "bindless 2d array" texture with 4 layers.
    once texture made resident - all layers reside in memory.
  - for terrain need to mix/blend potentially dozens of textures.

    I thought of using "sparse" texture here, to have single 100's layers texture
    which is partially loaded. (can describe it as "sparse bindless 2d array")
    each layer - different base terrain texture or additional "moisture/dirt/etc." to blend.

    But for now keep it simple - use "bindless 2D array" as for mesh,
    creating single Texture* object for each terrain base or additional texture
    and mix multiple texture handles.

    Also "Sparse" feature marked experimental in below resources, although links are a bit outdated.

Some usefull resources on the topic:

http://media.steampowered.com/apps/steamdevdays/slides/beyondporting.pdf

https://www.gamedev.net/forums/topic/683412-bindless-texturing-virtual-texture-and-array-texture/5317015/#:~:text=Bindless%20textures%20are%20a%20concept,same%20advantage%20as%20array%20textures.
     You're right, there's one missing thing - you forgot bindless textures.
     Bindless textures are a concept different from sparse textures -
     while sparse/virtual textures have potentially large textures that is only partly allocated,
     bindless textures will give you the same advantage as array textures.
     Basically, you have a texture and generate a global handle
     that you can use everywhere in your shaders without having to bind the texture.
     Before that, the texture is made resident.
     In your case you could generate a texture handle,
     save it in your global material buffer and afterwards referencing it via your material attributes,
     or write it to a texture of your gbuffer (not recommended).
     Instead of indexing into an array texture,
     where you can only have textures with the same size and format,
     you can use the handle directly and sample with the given uvs from the texture in a deferred way.

    To be clear, you could use sparse textures in combination with this approach.
    Even it doesn't make that much sense at first sight, you could make every single texture a sparse one.
    Virtual textures are nowadays mostly used for very large textures...
    for example you could combine all surface textures of a scene into a large one and make it sparse,
    with loading/unloading small pieces of it at runtime.
    But this brings other problems you don't want to have.
    I would recommend you to use bindless textures at first,
    I used it, it's simple to integrate if your hardware supports it and it gets the job done 100%.
    Performance overhead wouldnt matter that much, as I experienced similar experience as with array textures.


UPDATE 1 --------------------------------------------------------------------------------------

TODO TEXTURES MANAGING, TERRAIN INSPIRED:

    add to blinn_phong mesh uniforms "BlendTexturesArray" = [(texture_id, weight)]

    HAVE TO CHANGE:
        - texture array contains texture_id -> handle, not mesh_id -> handle.
        - Texture* contains ref count.
            Resident when used, release when unused
    ADVANTAGES:
        - to reuse blinn phong for terrain in single shader / draw call
        - to mix texture also for meshes.
            i.e. make models "sandy"/"dirty"/"wet"/"damaged"/"bleed",
            based on environment and effects applied.
                each model will have list of statuses, and each status -
                    texture;
                    blend percent weight;
                    mix function (i.e. dirt will be "mixed", damage - "overlayed/screened");

    ONCE FINISHED: GET RID OF TERRAIN SHADER FILES (shaders/shared/terrain_fs|vs.glsl, shaders/shared/_terrain.py, ...)
UPDATE 1 END --------------------------------------------------------------------------------------
*/

struct GLTextureHandles {

};

class TextureManager : public SausageSystem {
    unordered_map<unsigned int, unsigned int> texture_used_count_by_id;
    unordered_map<size_t, Texture*> texture_by_material_tex_names_hash;
    Samplers* samplers;
    BufferManager* buffer;
    ThreadSafeNumberPool* id_pool;
public:
  TextureManager(Samplers* samplers, BufferManager* buffer) :
    samplers{ samplers }, buffer{ buffer }, id_pool{ new ThreadSafeNumberPool(BufferSettings::MAX_TEXTURE) } {
  };
    /**
    * load texture array for mesh. diffuse + normal + height + specular.
    */
    Texture* LoadTextureArray(MaterialTexNames& tex_names);

    unique_ptr<RawTextureData> LoadRawTextureData(string& path);

    GLuint AllocateGLTextureId();

    /**
     * allocates texture id and handle, without buffered data
    */
    Texture* AllocateTextureWithHandle(GLuint texture_id, GLuint sampler);

    void Dispose(Texture* texture);
private:
    GLenum GetTexFormat(int bytes_per_pixel, bool for_storage);

    bool LoadLayer(string name, TextureType type);
};
