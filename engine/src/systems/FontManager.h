#pragma once

#include "sausage.h"
#include "AssetUtils.h"
#include "FontBufferConsumer.h"
#include "Logging.h"
#include <ft2build.h>
#include FT_FREETYPE_H

using namespace std;

struct Character {
    glm::ivec2   size;       // Size of glyph
    glm::ivec2   bearing;    // Offset from baseline to left/top of glyph
    unsigned int advance;    // Offset to advance to next glyph
};

class FontManager {
    FontBufferConsumer* buffer;
    Samplers* samplers;
    Texture* handle;
    std::map<char, Character> chars;

public:
	
    FontManager(Samplers* samplers, FontBufferConsumer* buffer) : samplers{ samplers }, buffer{ buffer } {};
	~FontManager() {};
    void Init() {

        InitFontTextures();
    }
    void InitFontTextures() {
        GLuint texture_id;
        FT_Library ft;
        if (FT_Init_FreeType(&ft))
        {
            std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            return;
        }
        FT_Face face;
        if (FT_New_Face(ft, GetFontPath("arial.ttf").c_str(), 0, &face))
        {
            std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
            return;
        }

        FT_Load_Char(face, 'X', FT_LOAD_RENDER);

        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id);
        glTexStorage3D(GL_TEXTURE_2D_ARRAY,
            1,                    //mipmaps. 1 == no mipmaps.
            GL_RED,              //Internal format
            face->glyph->bitmap.width, face->glyph->bitmap.rows,//width,height
            BufferSettings::TEXTURES_SINGLE_FONT            //Number of layers
        );
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
        CheckGLError();

        for (unsigned char c = 0; c < BufferSettings::TEXTURES_SINGLE_FONT; c++)
        {
            // load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // generate texture
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                0,                     //Mipmap number
                0, 0, c,               //xoffset, yoffset, zoffset
                face->glyph->bitmap.width, face->glyph->bitmap.rows, 1,               //width, height, depth
                GL_RED,                //format
                GL_UNSIGNED_BYTE,      //type
                face->glyph->bitmap.buffer);
            // now store character for later use
            Character character = {
                ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                face->glyph->advance.x
            };
            chars[c] = character;
        }
        CheckGLError();
        GLuint64 tex_handle = glGetTextureSamplerHandleARB(texture_id, samplers->font_sampler);
        CheckGLError();
        handle = new Texture(texture_id, tex_handle, {});
    }
};