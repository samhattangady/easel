#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in float time;
layout(location = 3) in vec4 inPos;

layout(location = 0) out vec4 outColor;

float rand(float n) { return fract(sin(n) * 43758.5453123); }
float rand(vec2 n) { return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453); }

float noise(float p){
	float fl = floor(p);
    float fc = fract(p);
	return mix(rand(fl), rand(fl + 1.0), fc);
}
	
float noise(vec2 n) {
	const vec2 d = vec2(0.0, 1.0);
    vec2 b = floor(n), f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
	return mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
}

vec2 rotate(vec2 point, float angle) {
    float cosa = cos(angle);
    float sina = sin(angle);
    return mat2(cosa, -sina, sina, cosa) * point;
}

void main() {
    vec2 texCoord = fragTexCoord;
    // TODO (07 Dec 2020 sam): See how this can be improved
    if (texCoord.x > 0) {
        texCoord = rotate(texCoord, distance(texCoord, vec2(0.0)) * sin(time*1.5) * noise(inPos.xy) * 3.1415/8.0f);
    }
    vec4 col = texture(texSampler, texCoord);
    if (col.a < 0.3)
        discard;
    outColor = col;
}
