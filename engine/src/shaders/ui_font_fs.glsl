#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require

in vs_out {
    flat int base_instance;
    // for non font glyph_id = -1
    flat int glyph_id;
    vec3 color;
    vec2 uv;
} In;

layout (std430, binding = 4) buffer FontTextureArray
{
    sampler2DArray glyphs[];
};

out vec4 color;

void main(void) {
  color = vec4(In.color, (In.glyph_id == -1) ? 1.0 : texture(glyphs[In.base_instance], vec3(In.uv, In.glyph_id)).r);
}
