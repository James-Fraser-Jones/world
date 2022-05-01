#version 330 core

in vec3 in_position;
in vec2 in_tex_coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec2 vert_tex_coord;

void main() {
    vert_tex_coord = in_tex_coord;
    gl_Position = proj * view * model * vec4(in_position, 1);
}