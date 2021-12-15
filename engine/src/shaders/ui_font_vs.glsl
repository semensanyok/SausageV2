#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_draw_parameters : require

// for ui font = (screen_x, screen_y, glyph_id)
// for non font = (screen_x, screen_y, -1)
layout (location = 0) in vec3 position_glyph_id;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 uv;

const uint MAX_TRANSFORM = 4000;

out vec2 TexCoords;

uniform mat4 projection_ortho;

layout (std430, binding = 5) buffer FontUniformDataUI
{
    vec2 transforms[MAX_TRANSFORM];
};


out vs_out {
    flat int base_instance;
    // for non font glyph_id == -1
    flat int glyph_id;
    vec3 color;
    vec2 uv;
} Out;

void main()
{
    vec2 transform = transforms[gl_BaseInstanceARB];
    gl_Position = projection_ortho * vec4(position_glyph_id.xy + transform, 0.0 , 1.0);
    Out.color = color;
    Out.glyph_id = int(position_glyph_id.z);
    Out.uv = uv.xy;
    Out.base_instance = gl_BaseInstanceARB;
}