#pragma once

class Shader;

enum ShaderType { BlinnPhong, BRDF };

struct Shaders {
  Shader* blinn_phong;
  Shader* outline;
  //Shader* stencil;
  Shader* font_ui;
  Shader* back_ui;
  Shader* mesh_static;
  Shader* terrain;
  //Shader* overlay_3d;
};
