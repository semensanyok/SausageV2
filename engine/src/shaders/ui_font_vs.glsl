#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_draw_parameters : require

layout (location = 0) in vec3 position_glyph_id;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 uv;

{{ ui_uniforms_shared }}
out vec2 TexCoords;

uniform mat4 projection_ortho;

out vs_out {
    flat int base_instance;
    flat int glyph_id;
    vec3 color;
    vec2 uv;
} Out;

void main()
{
    vec2 transform = transforms[gl_BaseInstanceARB];
    gl_Position = projection_ortho * vec4(position_glyph_id.xy + transform, 1.0 , 1.0);
    Out.color = color;
    Out.glyph_id = int(position_glyph_id.z);
    Out.uv = uv.xy;
    Out.base_instance = gl_BaseInstanceARB;
}