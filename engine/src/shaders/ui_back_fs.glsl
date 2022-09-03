#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require

in vs_out {
    flat int base_instance;
    vec3 color;
    vec2 uv;
} In;

{{ controller_uniforms_shared }}
{{ ui_uniforms_shared }}
{{ ui_utils }}
{{ uniforms_locations }}

layout (std430, binding = TEXTURE_UNIFORM_LOC) buffer TextureArray
{
    sampler2DArray textures[];
};

out vec4 color;

void main(void) {
  color = vec4(IsMouseHover(mouse_x, mouse_y, In.base_instance) ? In.color + 100 : In.color, 0.3);
}
