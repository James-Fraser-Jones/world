#version 330 core

in vec2 vert_color;
in vec2 vert_tex_coord;

uniform float blue_val;
uniform sampler2D sea_texture;

out vec4 out_color;

void main() {
    out_color = texture(sea_texture, vert_tex_coord);
    //out_color = vec4(vert_color, blue_val, 0.0f);
}