#pragma once

class Shader;

enum ShaderType { BlinnPhong, BRDF };

struct Shaders {
  Shader* blinn_phong;
  Shader* bullet_debug;
  Shader* stencil;
  Shader* font_ui;
  Shader* back_ui;
  //Shader* overlay_3d;
};
