#pragma once

#include <ft2build.h>

#include "AssetUtils.h"
#include "FontBufferConsumer.h"
#include "Logging.h"
#include "Renderer.h"
#include "Shader.h"
#include "sausage.h"
#include FT_FREETYPE_H

using namespace std;

namespace FontSizes {
const int STANDART = 48;
};

struct Character {
  glm::ivec2 size;       // Size of glyph
  glm::ivec2 bearing;    // Offset from baseline to left/top of glyph
  unsigned int advance;  // Offset to advance to next glyph
};

static struct BatchFontDataUI {
  MeshDataFontUI* mesh_data;
  vector<vec3> vertices;
  vector<vec3> colors;
  vector<vec2> uvs;
  vector<unsigned int> indices;
};
static struct BatchFontData3D {
  MeshDataFont3D* mesh_data;
  vector<vec3> vertices;
  vector<vec3> colors;
  vector<vec2> uvs;
  vector<unsigned int> indices;
  vector<vec3> glyph_id;
};

class FontManager {
  FontBufferConsumer* buffer;
  MeshManager* mesh_manager;
  DrawCall* draw_call_ui;
  DrawCall* draw_call_3d;
  Renderer* renderer;
  Samplers* samplers;
  Texture* handle;
  map<int, map<char, Character>> size_chars;
  // TODO: instancig
  // map<int, MeshDataFontUI> size_instanced_plane;

  const unsigned int command_buffer_size = 1;

  vector<MeshDataFontUI*> active_ui_texts;
  vector<MeshDataFont3D*> active_3d_texts;

  vector<BatchFontData3D> current_3d_batch;
  vector<BatchFontDataUI> current_ui_batch;
 public:
  FontManager(Samplers* samplers, FontBufferConsumer* buffer,
              MeshManager* mesh_manager, Renderer* renderer, Shaders* shaders)
      : samplers{samplers},
        buffer{buffer},
        mesh_manager{mesh_manager},
        renderer{renderer} {
    draw_call_ui = new DrawCall();
    draw_call_ui->shader = shaders->font_ui;
    draw_call_ui->mode = GL_TRIANGLES;
    draw_call_ui->buffer = (BufferConsumer*)buffer;
    draw_call_ui->command_buffer =
        draw_call_ui->buffer->CreateCommandBuffer(command_buffer_size);
    // TODO: 3d
     draw_call_3d = new DrawCall();
     draw_call_3d->shader = shaders->font_3d;
     draw_call_3d->mode = GL_TRIANGLES;
     draw_call_3d->buffer = (BufferConsumer*)buffer;
     draw_call_3d->command_buffer =
     draw_call_3d->buffer->CreateCommandBuffer(command_buffer_size);
  };
  ~FontManager(){};
  void Init() {
    _InitFontTextures();

    renderer->AddDraw(draw_call_ui);
    draw_call_ui->buffer->ActivateCommandBuffer(draw_call_ui->command_buffer);

    renderer->AddDraw(draw_call_3d);
    draw_call_3d->buffer->ActivateCommandBuffer(draw_call_3d->command_buffer);
  }
  void Deactivate() {
    renderer->RemoveDraw(draw_call_ui);
    draw_call_ui->buffer->RemoveCommandBuffer(draw_call_ui->command_buffer);

    renderer->RemoveDraw(draw_call_3d);
    draw_call_3d->buffer->RemoveCommandBuffer(draw_call_3d->command_buffer);
  }
  void WriteText3D(string& text, vec3 color, mat4& transform, float scale = 0.01) {
    BatchFontData3D batch;
    auto size_chars_pair = this->size_chars.begin();
    auto plate_size = size_chars_pair->first;
    auto charmap = size_chars_pair->second;

    float delta_x = 0;
    float delta_y = 0;
    int ind_delta = 0;
    for (auto chr : text) {
      auto character = charmap[chr];
      auto u = character.size.x * scale;
      auto v = character.size.y * scale;
      float uv_max_x = character.size.x / (float)plate_size;
      float uv_max_y = character.size.y / (float)plate_size;
      //batch.vertices.push_back({0 + delta_x, 0 + delta_y, 0.0});
      //batch.vertices.push_back({0 + delta_x, v + delta_y, 0.0});
      //batch.vertices.push_back({u + delta_x, 0 + delta_y, 0.0});
      //batch.vertices.push_back({u + delta_x, v + delta_y, 0.0});

      batch.vertices.push_back({0 + delta_x,0.0, 0 + delta_y});
      batch.vertices.push_back({0 + delta_x,0.0, v + delta_y});
      batch.vertices.push_back({u + delta_x,0.0, 0 + delta_y});
      batch.vertices.push_back({u + delta_x,0.0, v + delta_y});
      
      batch.colors.push_back(color);
      batch.colors.push_back(color);
      batch.colors.push_back(color);
      batch.colors.push_back(color);
      batch.uvs.push_back({0, uv_max_y});
      batch.uvs.push_back({0, 0});
      batch.uvs.push_back({uv_max_x, uv_max_y});
      batch.uvs.push_back({uv_max_x, 0});
      batch.glyph_id.push_back({chr, 0, 0});
      batch.glyph_id.push_back({chr, 0, 0});
      batch.glyph_id.push_back({chr, 0, 0});
      batch.glyph_id.push_back({chr, 0, 0});

      batch.indices.push_back(ind_delta + 1);
      batch.indices.push_back(ind_delta + 0);
      batch.indices.push_back(ind_delta + 2);
      batch.indices.push_back(ind_delta + 1);
      batch.indices.push_back(ind_delta + 2);
      batch.indices.push_back(ind_delta + 3);
      delta_x += (character.advance >> 6)* scale;
      ind_delta += 4;
    }
    batch.mesh_data = mesh_manager->CreateMeshDataFont3D(text, transform);
    current_3d_batch.push_back(batch);
    draw_call_3d->command_count = 1;
  }
  void WriteTextUI(string& text, vec3 color, float screen_x, float screen_y) {
    BatchFontDataUI batch;
    auto size_chars_pair = this->size_chars.begin();
    auto plate_size = size_chars_pair->first;
    auto charmap = size_chars_pair->second;

    float delta_x = 0;
    float delta_y = 0;
    int ind_delta = 0;
    for (auto chr : text) {
      auto character = charmap[chr];
      auto u = character.size.x;
      auto v = character.size.y;
      float uv_max_x = character.size.x / (float)plate_size;
      float uv_max_y = character.size.y / (float)plate_size;
      batch.vertices.push_back({0 + delta_x, 0 + delta_y, chr});
      batch.vertices.push_back({0 + delta_x, v + delta_y, chr});
      batch.vertices.push_back({u + delta_x, 0 + delta_y, chr});
      batch.vertices.push_back({u + delta_x, v + delta_y, chr});
      batch.colors.push_back(color);
      batch.colors.push_back(color);
      batch.colors.push_back(color);
      batch.colors.push_back(color);
      batch.uvs.push_back({0, uv_max_y});
      batch.uvs.push_back({0, 0});
      batch.uvs.push_back({uv_max_x, uv_max_y});
      batch.uvs.push_back({uv_max_x, 0});

      batch.indices.push_back(ind_delta + 1);
      batch.indices.push_back(ind_delta + 0);
      batch.indices.push_back(ind_delta + 2);
      batch.indices.push_back(ind_delta + 1);
      batch.indices.push_back(ind_delta + 2);
      batch.indices.push_back(ind_delta + 3);
      delta_x += character.advance >> 6;
      ind_delta += 4;
    }
    batch.mesh_data = mesh_manager->CreateMeshDataFontUI(text, vec2(screen_x, screen_y));
    current_ui_batch.push_back(batch);
    draw_call_ui->command_count = 1;
  }
  void PreUpdate() { flush(); }
  void flush() {
    _FlushUI();
    _Flush3D();
  }

 private:
  void _FlushUI() {
      for (int i = 0; i < current_ui_batch.size(); i++) {
      auto& batch = current_ui_batch[i];
      buffer->BufferMeshDataUI(batch.mesh_data, batch.vertices, batch.indices,
                               batch.colors, batch.uvs, {0, 0, 0},
                               handle);
      buffer->BufferTransform(batch.mesh_data);
      buffer->AddCommand(batch.mesh_data->command, draw_call_ui->command_buffer);
      active_ui_texts.push_back(batch.mesh_data);
    }
      current_ui_batch.clear();
  }
  void _Flush3D() {
      for (int i = 0; i < current_3d_batch.size(); i++) {
      auto& batch = current_3d_batch[i];
      buffer->BufferMeshData3D(batch.mesh_data, batch.vertices, batch.indices,
                               batch.colors, batch.uvs, batch.glyph_id, {0, 0, 0},
                               handle);
      buffer->BufferTransform(batch.mesh_data);
      buffer->AddCommand(batch.mesh_data->command, draw_call_3d->command_buffer);
      active_3d_texts.push_back(batch.mesh_data);
    }
      current_3d_batch.clear();
  }
  void _InitFontTextures() {
    this->size_chars.insert(
        pair<int, map<char, Character>>(FontSizes::STANDART, {}));

    GLuint texture_id;
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
      std::cout << "ERROR::FREETYPE: Could not init FreeType Library"
                << std::endl;
      return;
    }
    FT_Face face;
    if (FT_New_Face(ft, GetFontPath("arial.ttf").c_str(), 0, &face)) {
      std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
      return;
    }
    FT_Set_Pixel_Sizes(face, 0, FontSizes::STANDART);
    FT_Load_Char(face, 'A', FT_LOAD_RENDER);

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY,
                   1,                    // mipmaps. 1 == no mipmaps.
                   GL_R8,                // Internal format
                   FontSizes::STANDART,  // width
                   FontSizes::STANDART,  // height
                   // face->glyph->bitmap.width,//width
                   // face->glyph->bitmap.rows,//height
                   BufferSettings::TEXTURES_SINGLE_FONT  // Number of layers
    );
    glPixelStorei(GL_UNPACK_ALIGNMENT,
                  1);  // disable byte-alignment restriction
    CheckGLError();

    for (unsigned char c = 0; c < BufferSettings::TEXTURES_SINGLE_FONT; c++) {
      // load character glyph
      if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
        continue;
      }
      // generate texture
      glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                      0,        // Mipmap number
                      0, 0, c,  // xoffset, yoffset, zoffset
                      face->glyph->bitmap.width, face->glyph->bitmap.rows,
                      1,                 // width, height, depth
                      GL_RED,            // format
                      GL_UNSIGNED_BYTE,  // type
                      face->glyph->bitmap.buffer);
      CheckGLError();
      // now store character for later use
      Character character = {
          ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
          ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
          face->glyph->advance.x};
      this->size_chars[FontSizes::STANDART][c] = character;
    }
    CheckGLError();
    GLuint64 tex_handle =
        glGetTextureSamplerHandleARB(texture_id, samplers->font_sampler);
    CheckGLError();
    handle = new Texture(texture_id, tex_handle, {});
  }
};
