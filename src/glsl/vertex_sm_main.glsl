    shadowTexCoord = shadowPos.xy / shadowPos.w;
    shadowTexCoord = vec2(0.5) + (shadowTexCoord * 0.5);
    expectedDepth = shadowPos.z / shadowPos.w;
