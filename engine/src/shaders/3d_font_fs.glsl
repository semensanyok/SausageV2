#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require

in vs_out {
    vec2 uv;
    flat int base_instance;
    flat int glyph_id;
    vec3 glyph_color;
} In;

layout (std430, binding = 4) buffer FontTextureArray
{
    sampler2DArray glyphs;
};

out vec4 color;

void main(void) {
  color = vec4(In.glyph_color, texture(glyphs, vec3(In.uv, In.glyph_id)).r);
}
