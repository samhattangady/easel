#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    float depth = texture(texSampler, fragTexCoord).r;
    depth = min(1.0, depth);
    depth = smoothstep(0.19, 0.2, depth);
    // depth -= 0.99;
    // depth *= 100.0;
    outColor = vec4(depth, 0.5, 0.5, 1.0);
}
