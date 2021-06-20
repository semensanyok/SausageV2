#version 460

in vec3 v_color; 

out vec4 color;

void main(void) {
  color = vec4(v_color, 1.0);
}
