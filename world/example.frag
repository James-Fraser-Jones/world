#version 330 core

in vec2 vert_tex_coord;

uniform float mix_val;
uniform sampler2D sea_texture;
uniform sampler2D payday_texture;

out vec4 out_color;

void main() {
	out_color = mix(texture(sea_texture, vert_tex_coord), texture(payday_texture, vert_tex_coord), mix_val);
}