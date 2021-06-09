#version 460

in vec2 uv_out;
out vec4 color;

layout(binding=0) uniform sampler2D texture_diffuse;
layout(binding=1) uniform sampler2D texture_normal;
layout(binding=2) uniform sampler2D texture_specular;
layout(binding=3) uniform sampler2D texture_height;

void main(void) {
  color = texture2D(texture_diffuse, uv_out);
}
