#version 330
layout(location = 0) in vec3 uvVertex;
layout(location = 1) in vec3 posVertex;
out vec4 cols;
out vec2 uv;
uniform vec4 color;
void main() {
    gl_Position = vec4(posVertex, 1.0f);
    cols = color;
    uv = (1.0f + uvVertex.xy) / 2.0f;
}
