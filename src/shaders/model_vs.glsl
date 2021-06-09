#version 460

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;

uniform mat4 mvp;

out vec2 uv_out;

void main(void) {
  gl_Position = mvp * vec4(position, 1.0);
  uv_out = uv;
}
