#version 450

layout(location = 0) in vec3 g_frag_color;
layout(location = 0) out vec4 g_out_color;

void main() {
    g_out_color = vec4(g_frag_color, 1.0);
}
