#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;
uniform float time;
uniform float grainIntensity;

// Improved random function with better variation
float random(vec2 uv) {
    return fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec4 color = texture(texture0, fragTexCoord);

    // Convert to grayscale
    float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));

    // Generate film grain noise with better randomness
    float noise = random(fragTexCoord + vec2(time * 0.1, time * 0.2));
    noise = (noise - 0.5) * grainIntensity;

    // Apply noise to grayscale
    gray = clamp(gray + noise, 0.0, 1.0);

    finalColor = vec4(vec3(gray), color.a);
}
