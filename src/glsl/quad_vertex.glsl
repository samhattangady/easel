vec4 getPos() {
    vec3 pos = inPosition;
    return vec4(pos.xy*15.0, 14.9, 15.0);
}
