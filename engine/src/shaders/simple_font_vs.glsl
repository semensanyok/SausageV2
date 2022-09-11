#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_draw_parameters : require

{{ uniforms_locations }}
{{ ui_buffers_capacity }}

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
// for font shader use normal attrib pointer as glyph_id
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 glyph_id;

const uint MAX_BASE_MESHES = 4000;

out vec2 TexCoords;

uniform mat4 projection_view;

{{ 3d_overlay_uniforms }}

out vs_out {
    flat int base_instance;
    flat int glyph_id;
    vec3 color;
    vec2 uv;
} Out;

void main()
{
    mat4 transform = transforms[gl_BaseInstanceARB];
    gl_Position = projection_view * transform * vec4(position, 1.0);
    Out.color = color;
    Out.glyph_id = int(glyph_id.x);
    Out.uv = uv.xy;
    Out.base_instance = gl_BaseInstanceARB;
}