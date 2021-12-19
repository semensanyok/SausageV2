#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require

in vs_out {
    flat int base_instance;
    vec3 color;
    vec2 uv;
} In;

layout (std430, binding = 1) buffer TextureArray
{
    sampler2DArray textures[];
};

out vec4 color;

void main(void) {
  color = vec4(In.color, 0.3);
}
