vec4 getCol() {
    vec4 col = texture(texSampler, fragTexCoord);
    return vec4(1.0, 1.0, 1.0, col.a);
}
