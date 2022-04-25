#version 330 core

in vec3 in_position;
in vec2 in_color;
in vec2 in_tex_coord;

out vec2 vert_color;
out vec2 vert_tex_coord;

void main() {
    vert_color = in_color;
    vert_tex_coord = in_tex_coord;
    gl_Position = vec4(in_position, 1);
}