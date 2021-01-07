#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D shadowSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec2 shadowTexCoord;
layout(location = 3) in float expectedDepth;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 col = texture(texSampler, fragTexCoord);
    if (col.a < 0.3)
        discard;
    vec4 shadowCol = mix(vec4(0.0, 0.0, 0.0, 1.0), col, 0.2);
    float shadow = 0.0;
    if (shadowTexCoord.x >= 0.0 && shadowTexCoord.x <= 1.0 &&
        shadowTexCoord.y >= 0.0 && shadowTexCoord.y <= 1.0) {
        float shadowDepth = texture(shadowSampler, shadowTexCoord).x;
        if (shadowDepth > expectedDepth) {
            shadow = 0.0;
        } else
            shadow = 1.0;
    }
    col = mix(col, shadowCol, shadow);
    outColor = col;
}
