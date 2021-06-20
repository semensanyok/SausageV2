#version 460

layout (location = 0) in vec3 position;
// for bullet debug shader use normal attrib pointer as color
layout (location = 1) in vec3 color;

uniform mat4 projection_view;

out vec3 v_color; 

void main(void) {
  gl_Position = projection_view * vec4(position, 1.0);
  v_color = color;
}
