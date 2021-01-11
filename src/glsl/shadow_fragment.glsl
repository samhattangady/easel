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

float get_shadow(vec2 texCoord) {
    if (texCoord.x >= 0.0 && texCoord.x <= 1.0 &&
        texCoord.y >= 0.0 && texCoord.y <= 1.0) {
        float shadowDepth = texture(shadowSampler, texCoord).x;
        if (shadowDepth > expectedDepth)
            return 0.0;
        else
            return 1.0;
    }
    return 0.0;
}

void main() {
    vec4 col = texture(texSampler, fragTexCoord);
    if (col.a < 0.3)
        discard;
    vec4 shadowCol = mix(vec4(0.0, 0.0, 0.0, 1.0), col, 0.2);
    float shadow = 0.0;
    // TODO (11 Jan 2021 sam): Currently hardcoding the pixel size for antialiasing
    float x_pixel = 1.0 / 2048.0;
    float y_pixel = 1.0 / 2048.0;
    vec2 texCoord;
    for (int i=-1; i<=1; i++) {
        for (int j=-1; j<=1; j++) {
            texCoord = shadowTexCoord + vec2(x_pixel*i, y_pixel*j);
            shadow += get_shadow(texCoord);
        }
    }
    shadow /= 9.0;
    col = mix(col, shadowCol, shadow);
    outColor = col;
}
