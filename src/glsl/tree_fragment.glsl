
float rand(float n) { return fract(sin(n) * 43758.5453123); }
float rand(vec2 n) { return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453); }

float noise(float p){
	float fl = floor(p);
    float fc = fract(p);
	return mix(rand(fl), rand(fl + 1.0), fc);
}
	
float noise(vec2 n) {
	const vec2 d = vec2(0.0, 1.0);
    vec2 b = floor(n), f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
	return mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
}

vec2 rotate(vec2 point, float angle) {
    float cosa = cos(angle);
    float sina = sin(angle);
    return mat2(cosa, -sina, sina, cosa) * point;
}

vec4 getCol() {
    if (inColor.x > (time-1.0)/3.0)
        discard;
    if (inColor.y > max(0.001,(time-2.0))/3.0)
        discard;
    if (inColor.z > max(0.001,(time-3.5))/3.0)
        discard;
    vec2 texCoord = fragTexCoord;
    // TODO (07 Dec 2020 sam): See how this can be improved
    if (texCoord.x > 0) {
        texCoord = rotate(texCoord, distance(texCoord, vec2(0.0)) * sin(time/2.7) * noise(inPos.xz) * 3.1415/8.0f);
    }
    vec4 col = texture(texSampler, texCoord);
    // TODO (20 Jan 2021 sam): This needs to be based on distance from camera.
    // The closer it is, the higher the cutoff needs to be. Should scale from
    // ~0.5 to ~0.1.
    if (col.a < 0.1)
        discard;
    float leaf_tint = 1.0;
    if (texCoord.x > 0) {
        leaf_tint = inColor.z;
    }
    col = mix(col, vec4(col.xyz*0.7, 1.0), 1.0-leaf_tint);
    if (col.a < 0.3)
        discard;
    vec4 shadow = vec4(col.xyz * 0.2, 1.0);
    float light = dot(inLightDirection, normalize(inNormal));
    light = (light+1.0) / 2.0;
    col = mix(shadow, col, light);
    if (col.a < 0.3)
        col.a = 0.0;
    return col;
}

