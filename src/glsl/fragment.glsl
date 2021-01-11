vec4 getCol() {
    vec4 col = texture(texSampler, fragTexCoord);
    return col;
}

