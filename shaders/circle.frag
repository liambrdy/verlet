#version 330

layout (location = 0) out vec4 outColor;

in vec2 fragPosition;
in vec2 fragUv;
in vec4 fragColor;

uniform float radius;
uniform vec2 resolution;

void main() {
    float translatedRadius = radius*(resolution.x/resolution.y);
    float dist = distance(fragUv, vec2(0.5));
    float circle = 1 - smoothstep(translatedRadius-(translatedRadius*0.01), translatedRadius+(translatedRadius*0.01), dot(dist,dist)*4.0);
    outColor = vec4(circle) * fragColor;
}