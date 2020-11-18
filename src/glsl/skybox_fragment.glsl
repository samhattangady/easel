#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform samplerCube texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 coord = vec3(fragTexCoord.x, fragTexCoord.z, fragTexCoord.y);
    vec4 col = texture(texSampler, normalize(coord));
    if (col.a < 0.3)
        discard;
    outColor = col;
}
