#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require

{{ uniforms_locations }}

in vs_out {
    flat int base_instance;
    flat int glyph_id;
    vec3 color;
    vec2 uv;
} In;

// TODO: replace with common mesh texture array
// layout (std430, binding = FONT_TEXTURE_UNIFORM_LOC) buffer FontTextureArray
// {
//     sampler2DArray glyphs[];
// };

out vec4 color;

void main(void) {
  color = vec4(In.color, texture(glyphs[In.base_instance], vec3(In.uv, In.glyph_id)).r);
}
