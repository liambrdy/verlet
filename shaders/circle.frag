#version 330

layout (location = 0) out vec4 outColor;

in vec2 fragUv;
in vec4 fragColor;

void main() {
    outColor = fragColor;
}