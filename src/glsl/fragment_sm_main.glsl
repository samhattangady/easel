
    vec4 shadowCol = mix(vec4(0.0, 0.0, 0.0, 1.0), col, 0.2);
    float shadow = 0.0;
    // TODO (11 Jan 2021 sam): Currently hardcoding the pixel size for antialiasing
    float x_pixel = 1.0 / 2048.0;
    float y_pixel = 1.0 / 2048.0;
    vec2 texCoord;
    for (int i=-2; i<=2; i++) {
        for (int j=-2; j<=2; j++) {
            texCoord = shadowTexCoord + vec2(x_pixel*i, y_pixel*j);
            shadow += get_shadow(texCoord);
        }
    }
    shadow /= 25.0;
    col = mix(col, shadowCol, shadow);

