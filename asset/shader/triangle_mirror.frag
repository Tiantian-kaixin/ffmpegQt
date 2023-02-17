#version 330
in vec4 cols;
in vec2 uv;
out vec4 fragColor;
uniform sampler2D imgTexture;
uniform float width;

void main() {
    if (abs(uv.x + uv.y) < width) {
        fragColor = cols;
    } else {
        fragColor = texture(imgTexture, uv);
    }
}
