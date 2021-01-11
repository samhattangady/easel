#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in float time;
layout(location = 3) in vec3 inPos;
layout(location = 4) in vec3 inColor;
layout(location = 5) in vec3 inNormal;
layout(location = 6) in vec3 inLightDirection;

layout(location = 0) out vec4 outColor;

