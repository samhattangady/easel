vec4 getCol() {
    vec4 col = texture(texSampler, fragTexCoord);
    if (col.a < 0.3)
        col.a = 0.0;
    return col;
}

