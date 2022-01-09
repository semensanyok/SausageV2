#pragma once

#include <ft2build.h>

#include "AssetUtils.h"
#include "UIBufferConsumer.h"
#include "OverlayBufferConsumer3D.h"
#include "Logging.h"
#include "Renderer.h"
#include "Shader.h"
#include "sausage.h"
#include "OpenGLHelpers.h"
#include <limits>
#include <cstring>
#include FT_FREETYPE_H

using namespace std;

class FontManager;

namespace FontSizes {
  enum FontSizes {
    //STANDART = 24,
    STANDART = 48
  };
}

struct Character {
  glm::ivec2 size;       // Size of glyph
  FT_Vector advance;  // Offset to advance to next glyph
  unsigned int glyph_index;
  //FT_Glyph_Metrics metrics;
  int descend;
};

class BatchDataUI {
public:
  vector<vec3> vertices;
  vector<vec3> colors;
  vector<vec2> uvs;
  vector<unsigned int> indices;
  int x_min;
  int x_max;
  int y_min;
  int y_max;
  Texture* texture;
};
static struct BatchFontData3D {
  MeshDataOverlay3D* mesh_data;
  vector<vec3> vertices;
  vector<vec3> colors;
  vector<vec2> uvs;
  vector<unsigned int> indices;
  vector<vec3> glyph_id;
};

class FontManager {
  friend class ScreenOverlayManagerTest;
  //DrawCall* draw_call_3d;
  Samplers* samplers;
  map<int, pair<Texture*, map<char, Character>>> size_chars;
  const unsigned int command_buffer_size = 1;

  DrawCall* draw_call_3d;
  // TODO: free after all buttons / texts initialized.
  FT_Face ft_face;
 public:
  FontManager(Samplers* samplers,
    MeshManager* mesh_manager,
    Renderer* renderer,
    Shaders* shaders)
      : samplers{samplers} {
  };
  ~FontManager(){};
  void Init() {
    _InitFontTextures();
  }
  //void WriteText3D(string& text, vec3 color, mat4& transform) {
  //  BatchFontData3D batch;
  //  auto size_chars_pair = this->size_chars.begin();
  //  auto plate_size = size_chars_pair->first;
  //  auto charmap = size_chars_pair->second;

  //  float delta_x = 0;
  //  float delta_y = 0;
  //  int ind_delta = 0;
  //  for (auto chr : text) {
  //    auto character = charmap[chr];
  //    //auto u = character.size.x * font_scale;
  //    //auto v = character.size.y * font_scale;
  //    auto u = character.size.x;
  //    auto v = character.size.y;
  //    float uv_max_x = character.size.x / (float)plate_size;
  //    float uv_max_y = character.size.y / (float)plate_size;
  //    //batch.vertices.push_back({0 + delta_x, 0 + delta_y, 0.0});
  //    //batch.vertices.push_back({0 + delta_x, v + delta_y, 0.0});
  //    //batch.vertices.push_back({u + delta_x, 0 + delta_y, 0.0});
  //    //batch.vertices.push_back({u + delta_x, v + delta_y, 0.0});

  //                                               // -z forward
  //    batch.vertices.push_back({0 + delta_x,0.0, - 0 + delta_y});
  //    batch.vertices.push_back({0 + delta_x,0.0, - v + delta_y});
  //    batch.vertices.push_back({u + delta_x,0.0, - 0 + delta_y});
  //    batch.vertices.push_back({u + delta_x,0.0, - v + delta_y});
  //    
  //    batch.colors.push_back(color);
  //    batch.colors.push_back(color);
  //    batch.colors.push_back(color);
  //    batch.colors.push_back(color);
  //    batch.uvs.push_back({0, uv_max_y});
  //    batch.uvs.push_back({0, 0});
  //    batch.uvs.push_back({uv_max_x, uv_max_y});
  //    batch.uvs.push_back({uv_max_x, 0});
  //    batch.glyph_id.push_back({chr, 0, 0});
  //    batch.glyph_id.push_back({chr, 0, 0});
  //    batch.glyph_id.push_back({chr, 0, 0});
  //    batch.glyph_id.push_back({chr, 0, 0});

  //    batch.indices.push_back(ind_delta + 1);
  //    batch.indices.push_back(ind_delta + 0);
  //    batch.indices.push_back(ind_delta + 2);
  //    batch.indices.push_back(ind_delta + 1);
  //    batch.indices.push_back(ind_delta + 2);
  //    batch.indices.push_back(ind_delta + 3);
  //    //delta_x += (character.advance >> 6)* font_scale;
  //    delta_x += character.advance >> 6;
  //    ind_delta += 4;
  //  }
  //  batch.mesh_data = mesh_manager->CreateMeshDataFont3D(text, transform);
  //  current_3d_batch.push_back(batch);
  //  draw_call_3d->command_count = 1;
  //}
  unique_ptr<BatchDataUI> GetMeshTextUI(
    string& text,
    vec3 color,
    int font_size) {
    auto batch = make_unique<BatchDataUI>();
    auto size_chars_pair = this->size_chars.find(font_size);
    if (size_chars_pair == this->size_chars.end()) {
      LOG((ostringstream() << "size_chars does not contain requested font size: " << font_size).str());
      batch.release();
      return batch;
    }
    auto plate_size = size_chars_pair->first;
    batch->texture = size_chars_pair->second.first;
    auto charmap = size_chars_pair->second.second;

    int x_min = numeric_limits<int>::max();
    int x_max = numeric_limits<int>::min();
    int y_min = numeric_limits<int>::max();
    int y_max = numeric_limits<int>::min();

    int ind_delta = 0;

    int cursor_x = 0;
    int cursor_y = 0;
    for (int i = 0; i < text.size(); i++) {
      auto chr = text[i];
      auto character = charmap[chr];
      auto u = character.size.x;
      auto v = character.size.y;
      float uv_max_x = character.size.x / (float)plate_size;
      float uv_max_y = character.size.y / (float)plate_size;

      cursor_y -= character.descend;
      {
        x_min = std::min<int>(0 + cursor_x, x_min);
        x_max = std::max<int>(u + cursor_x, x_max);
        y_min = std::min<int>(0 + cursor_y, y_min);
        y_max = std::max<int>(v + cursor_y, y_max);

        batch->vertices.push_back({0 + cursor_x, 0 + cursor_y, chr});
        batch->vertices.push_back({0 + cursor_x, v + cursor_y, chr});
        batch->vertices.push_back({u + cursor_x, 0 + cursor_y, chr});
        batch->vertices.push_back({u + cursor_x, v + cursor_y, chr});
        batch->colors.push_back(color);
        batch->colors.push_back(color);
        batch->colors.push_back(color);
        batch->colors.push_back(color);
        batch->uvs.push_back({0, uv_max_y});
        batch->uvs.push_back({0, 0});
        batch->uvs.push_back({uv_max_x, uv_max_y});
        batch->uvs.push_back({uv_max_x, 0});

        batch->indices.push_back(ind_delta + 1);
        batch->indices.push_back(ind_delta + 0);
        batch->indices.push_back(ind_delta + 2);
        batch->indices.push_back(ind_delta + 1);
        batch->indices.push_back(ind_delta + 2);
        batch->indices.push_back(ind_delta + 3);
        ind_delta += 4;
      }
      cursor_y += character.descend;
      cursor_x += character.advance.x >> 6;
    }
    batch->x_max = x_max;
    batch->x_min = x_min;
    batch->y_max = y_max;
    batch->y_min = y_min;

    return batch;
  }

  void PreUpdate() { Flush(); }
  void Flush() {
    //_Flush3D();
  }

 private:
  //void _Flush3D() {
  //  fror (int i = 0; i < current_3d_batch.size(); i++) {
  //    auto& batch = current_3d_batch[i];
  //    buffer_3d->BufferMeshData(batch.mesh_data, batch.vertices, batch.indices,
  //                             batch.colors, batch.uvs, batch.glyph_id, {0, 0, 0},
  //                             font_texture);
  //    buffer_3d->BufferTransform(batch.mesh_data);
  //    buffer_3d->AddCommand(batch.mesh_data->command, draw_call_3d->command_buffer);
  //    active_3d_texts.push_back(batch.mesh_data);
  //  }
  //  current_3d_batch.clear();
  //}
  void _InitFontTextures() {
    _InitFontTexture(FontSizes::STANDART);
  }
  //void WriteTextCairo(string& text,
  //  vec3 color,
  //  int font_size) {
  //  auto width = text.size();
  //  auto height = font_size;
  //  auto stride = cairo_format_stride_for_width (CAIRO_FORMAT_A8, width);
  //  unsigned char* texture = (unsigned char*)malloc(stride * height);
  //  auto stride_bytes = stride / sizeof(char);
  //  memset(texture, 0, stride_bytes);
  //  auto cr = cairo_image_surface_create_for_data(texture, CAIRO_FORMAT_A8, width, height, stride);
  //  
  //}
  void _InitFontTexture(int font_size) {
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    CheckGLError();

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
      std::cout << "ERROR::FREETYPE: Could not init FreeType Library"
                << std::endl;
      return;
    }
    if (FT_New_Face(ft, GetFontPath("times.ttf").c_str(), 0, &ft_face)) {
      std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
      return;
    }
    FT_Set_Pixel_Sizes(ft_face, 0, font_size);
    FT_Load_Char(ft_face, 'A', FT_LOAD_RENDER);

    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY,
                   1,                    // mipmaps. 1 == no mipmaps.
                   GL_R8,                // Internal format
                   font_size,  // width
                   font_size,  // height
                   // face->glyph->bitmap.width,//width
                   // face->glyph->bitmap.rows,//height
                   BufferSettings::TEXTURES_SINGLE_FONT  // Number of layers
    );
    glPixelStorei(GL_UNPACK_ALIGNMENT,
                  1);  // disable byte-alignment restriction
    CheckGLError();

    for (unsigned char c = 0; c < BufferSettings::TEXTURES_SINGLE_FONT; c++) {
      // load character glyph
      if (FT_Load_Char(ft_face, c, FT_LOAD_RENDER)) {
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
        continue;
      }
      // generate texture
      glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                      0,        // Mipmap number
                      0, 0, c,  // xoffset, yoffset, zoffset
                      ft_face->glyph->bitmap.width, ft_face->glyph->bitmap.rows,
                      1,                 // width, height, depth
                      GL_RED,            // format
                      GL_UNSIGNED_BYTE,  // type
                      ft_face->glyph->bitmap.buffer);
      CheckGLError();
      // now store character for later use
      Character character = {
          ivec2(ft_face->glyph->bitmap.width, ft_face->glyph->bitmap.rows),
          ft_face->glyph->advance,
          ft_face->glyph->glyph_index,
          //ft_face->glyph->metrics,
          (ft_face->glyph->metrics.height - ft_face->glyph->metrics.horiBearingY) >> 6};
      this->size_chars[font_size].second[c] = character;
    }
    GLuint64 tex_handle =
        glGetTextureSamplerHandleARB(texture_id, samplers->font_sampler);
    auto texture = new Texture(texture_id, tex_handle, {});
    this->size_chars[font_size].first = texture;
    CheckGLError();
  }
};
