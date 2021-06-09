#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require

out vec4 color;

in vs_out {
    flat int draw_id_arb;
    vec2 uv;
} In;

// Texture block
layout (std430, binding = 1) buffer TextureArray
{
    sampler2DArray tex[];
};

void main(void) {
  // normal = texture(textures, vec3(In.uv,1));
  color = texture(tex[In.draw_id_arb], vec3(In.uv, 0));
}
