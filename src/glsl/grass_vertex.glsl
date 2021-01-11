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

mat4 rotation_matrix_axis(float angle, vec3 axis) {
    float cosa = cos(angle);
    float sina = sin(angle);
    vec3 n = normalize(axis);
    mat4 rotation = mat4(
        n.x*n.x*(1.0-cosa) + cosa,
        n.x*n.y*(1.0-cosa) - n.z*sina,
        n.x*n.z*(1.0-cosa) + n.y*sina,
        0.0,
        n.x*n.y*(1.0-cosa) + n.z*sina,
        n.y*n.y*(1.0-cosa) + cosa,
        n.y*n.z*(1.0-cosa) - n.x*sina,
        0.0,
        n.x*n.z*(1.0-cosa) - n.y*sina,
        n.y*n.z*(1.0-cosa) + n.x*sina,
        n.z*n.z*(1.0-cosa) + cosa,
        0.0,
        0.0, 0.0, 0.0, 1.0
    );
    return rotation;
}

vec4 getPos() {
    vec4 pos = vec4(inPosition, 1.0f);
    vec4 base_obj_pos = vec4(inColor, 1.0f);
    vec4 obj_pos = vec4(inColor, 1.0f);
    obj_pos.x += (1.0-inTexCoord.y)/5.0 * sin(ubo.time*cos(noise(vec2(pos.x, pos.y)))*1.7);
    obj_pos.y += (1.0-inTexCoord.y)/8.0 * abs(sin(ubo.time*1.3*pos.y*0.5));
    vec3 pos_to_cam = normalize(vec3(ubo.camera_position.x, 0, ubo.camera_position.z) - vec3(pos.x, 0, pos.z));
    float angle_to_camera = acos(dot(pos_to_cam, inNormal));
    // acos only works from 0 to pi
    if (pos_to_cam.x > inNormal.x)
        angle_to_camera *= -1.0;
    // angle_to_camera = ubo.time;
    mat4 rotation_mat = rotation_matrix_axis(angle_to_camera, vec3(0,1.0,0));
    obj_pos = rotation_mat * obj_pos;
    pos += obj_pos-base_obj_pos;
    return pos;
}
