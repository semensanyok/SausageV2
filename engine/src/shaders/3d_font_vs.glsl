#version 460

layout (location = 0) in vec3 position;
// for font shader use normal attrib pointer as glyph_id
layout (location = 1) in vec3 glyph_id;
layout (location = 2) in vec3 glyph_color;

const uint MAX_TRANSFORM = 4000;
const uint MAX_TRANSFORM_OFFSET = MAX_TRANSFORM * 10;

out vec2 TexCoords;

layout (std430, binding = 3) buffer FontUniformData
{
    mat4 transforms[MAX_TRANSFORM];
    unsigned int transform_offset[MAX_TRANSFORM_OFFSET];
};

out vs_out {
    int base_instance;
    vec3 frag_pos;
    vec3 glyph_id;
    vec3 glyph_color;
} Out;

void main()
{
    gl_Position = transforms[transform_offset[gl_BaseInstanceARB] + gl_InstanceID];;
    vec4 res_position = vec4(position, 1.0);
    Out.frag_pos = vec3(transform * res_position);
    Out.glyph_color = glyph_color;
}