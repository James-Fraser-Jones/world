#version 330 core

in vec2 vert_color;
uniform float blue_val;
out vec4 out_color;

void main() {
    out_color = vec4(vert_color, blue_val, 0.0f);
}