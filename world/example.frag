#version 330 core

out vec4 out_color;

uniform float green_val;

void main() {

    out_color = vec4(0.3f, green_val, 0.55f, 0.0f);

}