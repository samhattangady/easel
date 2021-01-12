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
layout(location = 2) out float time;
layout(location = 3) out vec3 outPos;
layout(location = 4) out vec3 outColor;
layout(location = 5) out vec3 outNormal;
layout(location = 6) out vec3 lightDirection;

vec4 getPos() {
    vec4 pos = vec4(inPosition, 0.0);
    pos.x = ((pos.x/ubo.window_size.x) * 2.0f) - 1.0f;
    pos.y = ((pos.y/ubo.window_size.y) * 2.0f) - 1.0f;
    return vec4(pos.xy*10, 1.0, 10.0);
}

void main() {
    vec4 pos = getPos();
    gl_Position = pos;
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    time = ubo.time;
    outPos = pos.xyz;
    outColor = inColor;
    outNormal = inNormal;
    lightDirection = ubo.light_direction;
}



