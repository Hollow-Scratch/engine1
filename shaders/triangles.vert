#version 450

vec2 g_positions[3] = vec2[](
    vec2( 0.0, -0.5), // Top (Red)
    vec2( 0.5,  0.5), // Bottom Right (Green)
    vec2(-0.5,  0.5)  // Bottom Left (Blue)
);

vec3 g_colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

layout(location = 0) out vec3 g_frag_color;

void main() {
    gl_Position = vec4(g_positions[gl_VertexIndex], 0.0, 1.0);
    g_frag_color = g_colors[gl_VertexIndex];
}
