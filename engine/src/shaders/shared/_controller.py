controller_uniforms = {"controller_uniforms_shared" :
"""
layout (std430, binding = 6) buffer ControllerUniformData
{
  int mouse_x;
  int mouse_y;
  int is_pressed;
  int is_click;
};
"""
}