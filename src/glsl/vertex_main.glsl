
void main() {
    vec4 pos = getPos();
    vec4 shadowPos = ubo.light_proj * pos;
    if (ubo.state == 0)
        gl_Position = shadowPos;
    else if (ubo.state == 1)
        gl_Position = ubo.proj * ubo.view * ubo.model * pos;
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    time = ubo.time;
    outPos = pos.xyz;
    outColor = inColor;
    outNormal = inNormal;
    lightDirection = ubo.light_direction;

