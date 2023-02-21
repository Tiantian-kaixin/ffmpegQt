#version 330
in vec4 cols;
in vec2 uv;
out vec4 fragColor;
uniform sampler2D imgTexture;
uniform float width;

vec2 RGB_To_MyCbCr(vec3 rgb) {
    float Cb = rgb.b - rgb.g;
    float Cr = rgb.r - rgb.g;
    return vec2(Cb, Cr) * 0.5 + 0.5;
}

vec3 RGB_To_YCbCr(vec3 rgb) {
    float Y = 0.299 * rgb.r + 0.587 * rgb.g + 0.114 * rgb.b;
    float Cb = 0.564 * (rgb.b - Y);
    float Cr = 0.713 * (rgb.r - Y);
    return vec3(Cb, Cr, Y);
}

void main() {
    vec3 colors = RGB_To_YCbCr(texture(imgTexture, uv).rgb);
    if (colors.x <= 0 && colors.y <= 0) {
        fragColor = vec4(1);
    } else {
        fragColor = texture(imgTexture, uv);
    }
}
