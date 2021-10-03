#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_draw_parameters : require

layout (location = 0) in vec3 position;
// for font shader use normal attrib pointer as glyph_id
layout (location = 1) in vec3 glyph_id;
layout (location = 2) in vec3 glyph_color;

const uint MAX_TRANSFORM = 4000;
const uint MAX_TRANSFORM_OFFSET = MAX_TRANSFORM * 10;

out vec2 TexCoords;

uniform mat4 projection_view;

layout (std430, binding = 3) buffer FontUniformData
{
    mat4 transforms[MAX_TRANSFORM];
    unsigned int transform_offset[MAX_TRANSFORM_OFFSET];
};

out vs_out {
    int base_instance;
    int glyph_id;
    vec3 glyph_color;
} Out;

void main()
{
    mat4 transform = transforms[transform_offset[gl_BaseInstanceARB] + gl_InstanceID];
    gl_Position = projection_view * transform * vec4(position, 1.0);
    Out.glyph_color = glyph_color;
    Out.glyph_id = int(glyph_id.x);
}