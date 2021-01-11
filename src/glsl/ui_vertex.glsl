vec4 getPos() {
    vec4 pos = vec4(inPosition, 0.0);
    pos.x = ((pos.x/ubo.window_size.x) * 2.0f) - 1.0f;
    pos.y = ((pos.y/ubo.window_size.y) * 2.0f) - 1.0f;
    return vec4(pos.xy*10, 1.0, 10.0);
}
