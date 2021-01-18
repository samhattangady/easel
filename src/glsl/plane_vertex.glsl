

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

mat4 look_at(vec3 eye, vec3 target, vec3 up) {
    vec3 zaxis = normalize(eye-target);
    vec3 xaxis = normalize(cross(up, zaxis));
    vec3 yaxis = normalize(cross(zaxis, xaxis));
    return mat4(xaxis.x, yaxis.x, zaxis.x, 0.0,
                xaxis.y, yaxis.y, zaxis.y, 0.0,
                xaxis.z, yaxis.z, zaxis.z, 0.0,
                -dot(xaxis, eye), -dot(yaxis, eye), -dot(zaxis, eye), 1.0);
}

// Function to get the mat4 to align z along d
// https://iquilezles.org/www/articles/noacos/noacos.htm
mat4 rotation_align(vec3 z, vec3 d) {
    vec3 v = cross(z, d);
    float c = dot(z, d);
    float k = 1.0/(1.0+c);

    return mat4(v.x*v.x*k + c,     v.y*v.x*k - v.z,    v.z*v.x*k + v.y, 0.0,
                v.x*v.y*k + v.z,   v.y*v.y*k + c,      v.z*v.y*k - v.x, 0.0,
                v.x*v.z*k - v.y,   v.y*v.z*k + v.x,    v.z*v.z*k + c  , 0.0,
                0.0,               0.0,                0.0,             1.0);
}

// This approach works, but doesnot handle rotations along the forward axis
// Also behaves wierdly arond the 180 deg mark.
vec4 getPosTrig() {
    vec3 xaxis = vec3(ubo.view[0].x, ubo.view[1].x, ubo.view[2].x);
    vec3 yaxis = vec3(ubo.view[0].y, ubo.view[1].y, ubo.view[2].y);
    vec3 zaxis = vec3(ubo.view[0].z, ubo.view[1].z, ubo.view[2].z);
    vec3 up = vec3(0.0, 1.0, 0.0);  // Default orientation of the model.
    vec3 forward = vec3(0.0, 0.0, 1.0);  // Default orientation of the model.
    vec3 local = inPosition;

    mat4 rot_align = rotation_align(normalize(zaxis), forward);
    local = (rot_align * vec4(local, 1.0)).xyz;
    vec3 new_y = (rot_align * vec4(yaxis, 1.0)).xyz;
    vec3 new_z = (rot_align * vec4(zaxis, 1.0)).xyz;
    mat4 up_align = rotation_align(normalize(yaxis), up);
    local = (up_align * vec4(local, 1.0)).xyz;
    vec3 translation = -5.0 * zaxis + -0.4*new_y;

    vec4 pos = vec4(ubo.camera_position + local + translation, 1.0);
    return pos;
}

vec4 getPosLookAt() {
    vec3 xaxis = vec3(ubo.view[0].x, ubo.view[1].x, ubo.view[2].x);
    vec3 yaxis = vec3(ubo.view[0].y, ubo.view[1].y, ubo.view[2].y);
    vec3 zaxis = vec3(ubo.view[0].z, ubo.view[1].z, ubo.view[2].z);
    vec3 up      = vec3(0.0, 1.0, 0.0);   // Default orientation of the model.
    vec3 forward = vec3(0.0, 0.0, 1.0);   // Default orientation of the model.
    vec3 local = inPosition;

    vec3 reflection_normal = normalize(cross(up, forward));
    vec3 new_target = zaxis + 2.0*dot(zaxis, reflection_normal) * reflection_normal;

    // mat4 rotmat = mat4( xaxis.x,  yaxis.x,  zaxis.x,  0.0,
    //                     xaxis.y,  yaxis.y,  zaxis.y,  0.0,
    //                     xaxis.z,  yaxis.z,  zaxis.z,  0.0,
    //                     0.0,      0.0,      0.0,      1.0);
    mat4 rotmat = look_at(vec3(0.0), new_target, yaxis);
    vec3 new_y = (rotmat * vec4(yaxis, 1.0)).xyz;
    local = (rotmat * vec4(local, 1.0)).xyz;
    vec3 translation = -5.0 * zaxis; // + -0.4*new_y;

    // vec4 pos = vec4(ubo.camera_position + local + translation, 1.0);
    vec4 pos = vec4(local + vec3(0.0, 8.0, 0.0), 1.0);
    return pos;
}

vec4 getPos() {
    return vec4(inPosition, 1.0);
}
