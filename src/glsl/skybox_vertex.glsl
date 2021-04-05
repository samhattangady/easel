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
layout(location = 1) out vec3 fragTexCoord;  // This is different for skybox

void main() {
    vec3 pos = inPosition + ubo.camera_position;
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(pos, 1.0);
    fragColor = inColor;
    fragTexCoord = inPosition;
}

