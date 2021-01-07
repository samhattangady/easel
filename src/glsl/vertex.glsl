#version 450
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 light_proj;
    vec3 camera_position;
    float time;
    vec3 light_direction;
    float padding;
    vec2 window_size;
    int state;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec4 inOthers;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec2 shadowTexCoord;
layout(location = 3) out float expectedDepth;

void main() {
    vec4 pos = vec4(inPosition, 1.0f);
    vec4 shadowPos = ubo.light_proj * pos;
    if (ubo.state == 0)
        gl_Position = shadowPos;
    else if (ubo.state == 1)
        gl_Position = ubo.proj * ubo.view * ubo.model * pos;
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    shadowTexCoord = shadowPos.xy / shadowPos.w;
    shadowTexCoord = vec2(0.5) + (shadowTexCoord * 0.5);
    expectedDepth = shadowPos.z / shadowPos.w;
}
