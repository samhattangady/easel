#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in float time;
layout(location = 3) in vec3 inPos;
layout(location = 4) in vec3 inColor;

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
    if (inColor.x > (time-1.0)/3.0)
        discard;
    if (inColor.y > max(0.001,(time-2.0))/3.0)
        discard;
    if (inColor.z > max(0.001,(time-3.5))/3.0)
        discard;
    vec2 texCoord = fragTexCoord;
    // TODO (07 Dec 2020 sam): See how this can be improved
    // if (texCoord.x > 0) {
    //     texCoord = rotate(texCoord, distance(texCoord, vec2(0.0)) * sin(time*1.7) * noise(inPos.xz) * 3.1415/8.0f);
    // }
    vec4 col = texture(texSampler, texCoord);
    if (col.a < 0.3)
        discard;
    float leaf_tint = 1.0;
    if (texCoord.x > 0) {
        leaf_tint = inColor.z;
    }
    col = mix(col, col*0.5, 1.0-leaf_tint);
    // col = vec4(inColor, 1.0);
    outColor = col;
}
