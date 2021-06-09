#version 460

#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_image_load_store : require

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
  color = vec4((In.draw_id_arb + 1)*255.0, 0, 0.0, 1.0);
  // color = texture(tex[0], vec3(0.5, 0.5, 0));
  //color = texture(tex[0], vec3(0.5, 0.5, 1)); //normal
}
