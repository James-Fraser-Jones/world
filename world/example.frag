#version 330 core

in vec2 vert_color;
in vec2 vert_tex_coord;

uniform float blue_val;
uniform sampler2D sea_texture;
uniform sampler2D oil_texture;

out vec4 out_color;

const vec3 brightness = vec3(0.3f,0.59f,0.11f);

void main() {
//    vec3 tex = texture(sea_texture, vert_tex_coord).rgb;
//    float grey_scale = dot(tex, brightness);
//    vec3 grey = vec3(1.0f, 1.0f, 1.0f) * grey_scale;
//    vec3 mixer = grey * vec3(vert_color, blue_val);
//    out_color = vec4(mixer, 0.0f);

	out_color = texture(oil_texture, vert_tex_coord); //mix(texture(sea_texture, vert_tex_coord),  , 0.5);
}