
layout(binding = 2) uniform sampler2D shadowSampler;

layout(location = 7) in vec2 shadowTexCoord;
layout(location = 8) in float expectedDepth;

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

