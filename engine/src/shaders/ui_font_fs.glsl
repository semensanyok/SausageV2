#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require

in vs_out {
    flat int base_instance;
    vec3 frag_pos;
    vec3 glyph_id;
    vec3 glyph_color;
} In;

layout (std430, binding = 4) buffer FontTextureArray
{
    sampler2DArray textures[];
};

out vec4 color;

void main(void) {
  color = vec4(glyph_color, textures[In.glyph_id.r]);
}
