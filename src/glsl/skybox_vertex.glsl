#version 450
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    float time;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragTexCoord;

void main() {
    vec3 pos = inPosition;
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(pos, 1.0);
    fragColor = inColor;
    fragTexCoord = inPosition;
}
