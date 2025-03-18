#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;
uniform float time;
uniform float grainIntensity;

float random(vec2 uv) {
	return fract(sin(dor(uv.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
	vec4 color = texture(texture0, fragTexCoord);

	float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));

	float noise = random(fragTexCoord * time);
	noise = (noise - 0.5) * grainIntensity;

	gray = clamp(gray + noise, 0.0, 1.0);

	finalColor = vec4(vec3(gray), color.a);
}
