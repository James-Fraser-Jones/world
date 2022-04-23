#version 330 core

in vec3 in_position;
in vec2 in_color;
uniform float shift_right;
out vec2 vert_color;

void main() {
    vert_color = in_color;
    gl_Position = vec4(in_position.x + shift_right, -in_position.y, in_position.z, 1);
}