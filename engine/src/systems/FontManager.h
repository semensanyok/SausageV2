#pragma once

#include "sausage.h"
#include "AssetUtils.h"
#include "FontBufferConsumer.h"
#include "Logging.h"
#include "Shader.h"
#include "Renderer.h"
#include <ft2build.h>
#include FT_FREETYPE_H

using namespace std;

namespace FontSizes {
    const int STANDART = 48;
};

struct Character {
    glm::ivec2   size;       // Size of glyph
    glm::ivec2   bearing;    // Offset from baseline to left/top of glyph
    unsigned int advance;    // Offset to advance to next glyph
};

class FontManager {
    FontBufferConsumer* buffer;
    DrawCall* draw_call_ui;
    DrawCall* draw_call_3d;
    Renderer* renderer;
    Samplers* samplers;
    Texture* handle;
    map<int, map<char, Character>> size_chars;

    const unsigned int command_buffer_size = 1;

    vector<vec3> vertices;
    vector<vec3> colors;
    vector<vec2> uvs;
    vector<unsigned int> indices;
public:
	
    FontManager(Samplers* samplers,
        FontBufferConsumer* buffer,
        Renderer* renderer,
        Shaders* shaders) : samplers{ samplers }, buffer{ buffer }, renderer{ renderer } {
        draw_call_ui = new DrawCall();
        draw_call_ui->shader = shaders->font_ui;
        draw_call_ui->mode = GL_TRIANGLES;
        draw_call_ui->buffer = (BufferConsumer*)buffer;
        draw_call_ui->command_buffer = draw_call_ui->buffer->CreateCommandBuffer(command_buffer_size);
        draw_call_ui->command_count = 1;
        // TODO: 3d
        // draw_call_3d = new DrawCall();
        // draw_call_3d->shader = shaders->font_3d;
        // draw_call_3d->mode = GL_TRIANGLES;
        // draw_call_3d->buffer = (BufferConsumer*)buffer;
        // draw_call_3d->command_buffer = draw_call_3d->buffer->CreateCommandBuffer(command_buffer_size);
    };
	~FontManager() {};
    void Init() {
        _InitFontTextures();
        renderer->AddDraw(draw_call_ui);
        draw_call_ui->buffer->ActivateCommandBuffer(draw_call_ui->command_buffer);
        
        // TODO: 3d
        //renderer->AddDraw(draw_call_3d);
        //draw_call_3d->buffer->ActivateCommandBuffer(draw_call_3d->command_buffer);
    }
    void Deactivate() {
        renderer->RemoveDraw(draw_call_ui);
        draw_call_ui->buffer->RemoveCommandBuffer(draw_call_ui->command_buffer);

        // TODO: 3d
        //renderer->RemoveDraw(draw_call_3d);
        //draw_call_3d->buffer->RemoveCommandBuffer(draw_call_3d->command_buffer);
    }
    void WriteTextUI(string& text, int screen_x, int screen_y) {
        // TODO: make each font letter instanced mesh.
        auto charmap = this->size_chars.begin()->second;
        for (auto chr : text) {
            auto character = charmap['A'];
            auto u = character.size.x;
            auto v = character.size.y;
            vertices.push_back({0,0,'A'});
            vertices.push_back({0,v,'A'});
            vertices.push_back({u,0,'A'});
            vertices.push_back({u,v,'A'});
            colors.push_back({ 255, 0, 0 });
            colors.push_back({ 255, 0, 0 });
            colors.push_back({ 255, 0, 0 });
            colors.push_back({ 255, 0, 0 });
            // TODO: fix uv order?
            uvs.push_back({0,0});
            uvs.push_back({0,1});
            uvs.push_back({1,0});
            uvs.push_back({1,1});

            indices.push_back(1);
            indices.push_back(0);
            indices.push_back(2);
            indices.push_back(1);
            indices.push_back(2);
            indices.push_back(3);
            break;  
        }
    }
    void PreUpdate() {
        flush();
    }
    void flush() {
        auto mesh_data = buffer->BufferMeshDataUI(vertices, indices, colors, uvs, { 0,0,0 }, handle);
        buffer->AddCommand(mesh_data->command, draw_call_ui->command_buffer);
        //vertices.clear();
        //indices.clear();
        //colors.clear();
        //uvs.clear();
    }
private:
    void _InitFontTextures() {
        this->size_chars.insert(pair<int, map<char, Character>>(FontSizes::STANDART, {}));

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
        FT_Set_Pixel_Sizes(face, 0, FontSizes::STANDART);
        FT_Load_Char(face, 'A', FT_LOAD_RENDER);

        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id);
        glTexStorage3D(GL_TEXTURE_2D_ARRAY,
            1,                    //mipmaps. 1 == no mipmaps.
            GL_R8,              //Internal format
            FontSizes::STANDART,//width
            FontSizes::STANDART,//height
            //face->glyph->bitmap.width,//width
            //face->glyph->bitmap.rows,//height
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
            CheckGLError();
            // now store character for later use
            Character character = {
                ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                face->glyph->advance.x
            };
            this->size_chars[FontSizes::STANDART][c] = character;
        }
        CheckGLError();
        GLuint64 tex_handle = glGetTextureSamplerHandleARB(texture_id, samplers->font_sampler);
        CheckGLError();
        handle = new Texture(texture_id, tex_handle, {});

        buffer->BufferFontTextureHandle(handle);
    }
};