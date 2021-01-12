
vec4 getCol() {
    vec4 col = texture(texSampler, fragTexCoord.xy);
    // TODO (11 Jan 2021 sam): We might not want to discard for this one specifically.
    // So might have to figure out how to arrange the pipeline to support that one.
    if (col.a < 0.9)
        col.a = 0.0;
    return col;
}


