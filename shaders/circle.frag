#version 330

layout (location = 0) out vec4 outColor;

in vec2 fragUv;
in vec4 fragColor;

void main() {
    float dist = distance(fragUv - vec2(.5), vec2(0.0));
    float delta = 0.01;
    float alpha = 1.0 - smoothstep(0.5-delta, 0.5, dist);
    outColor = vec4(fragColor.rgb, fragColor.a * alpha);
}