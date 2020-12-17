#include "SDL.h"
#include "es_geometrygen.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"


float _geom_error_from_lod(Uint32 lod);
Uint32 _geom_get_vertices_from_radius(float radius, Uint32 lod);
Uint32 _geom_remaining_vertices(EsGeometry* geom);
Uint32 _geom_remaining_faces(EsGeometry* geom);
Uint32 _geom_remaining_textures(EsGeometry* geom);
Uint32 _geom_remaining_colors(EsGeometry* geom);
Uint32 _geom_remaining_normals(EsGeometry* geom);

typedef struct {
    vec3 key;
    int value;
} Vec3Map;

typedef struct {
    vec2 key;
    int value;
} Vec2Map;

SDL_bool geom_simplify_geometry(EsGeometry* geom) {
    // We want to remove all the duplicate vertices, textures and normals.
    // The way we do this is by using hashmaps.
    // So we create a hashmap for vertices, textures and normals, with key as vert
    // and value as index. (where while inserting, we only increment index if the
    // element doesn't already exist in the hashmap)
    // Once all the hashmaps have been filled, we fill simplified with all the vertices
    // as per their indexvalue in the hashmap. For each face, we query the hashmap for
    // the simplified index, and load that in.
    int vertex_index = 0;
    int texture_index = 0;
    int normal_index = 0;
    int color_index = 0;
    Vec3Map* vertices = NULL;
    Vec2Map* textures = NULL;
    Vec3Map* normals = NULL;
    Vec3Map* colors = NULL;
    hmdefault(vertices, -1);
    hmdefault(textures, -1);
    hmdefault(normals, -1);
    hmdefault(colors, -1);
    for (Uint32 i=0; i<geom->num_vertices; i++) {
        vec3 vert = geom->vertices[i];
        if (hmget(vertices, vert) < 0) {
            hmput(vertices, vert, vertex_index);
            vertex_index++;
        }
    }
    SDL_Log("total vertices = %i, unique vertices = %i\n", geom->num_vertices, vertex_index);
    for (Uint32 i=0; i<geom->num_textures; i++) {
        vec2 tex = geom->textures[i];
        if (hmget(textures, tex) < 0) {
            hmput(textures, tex, texture_index);
            texture_index++;
        }
    }
    SDL_Log("total textures = %i, unique textures = %i\n", geom->num_textures, texture_index);
    for (Uint32 i=0; i<geom->num_normals; i++) {
        vec3 norm = geom->normals[i];
        if (hmget(normals, norm) < 0) {
            hmput(normals, norm, normal_index);
            normal_index++;
        }
    }
    SDL_Log("total normals = %i, unique normals = %i\n", geom->num_normals, normal_index);
    for (Uint32 i=0; i<geom->num_colors; i++) {
        vec3 col = geom->colors[i];
        if (hmget(colors, col) < 0) {
            hmput(colors, col, color_index);
            color_index++;
        }
    }
    SDL_Log("total colors = %i, unique colors = %i\n", geom->num_colors, color_index);
    EsGeometry simple = geom_init_geometry_size(vertex_index, geom->num_faces, texture_index, normal_index, color_index);
    for (Uint32 i=0; i<geom->num_vertices; i++)
        simple.vertices[hmget(vertices, geom->vertices[i])] = geom->vertices[i];
    for (Uint32 i=0; i<geom->num_textures; i++)
        simple.textures[hmget(textures, geom->textures[i])] = geom->textures[i];
    for (Uint32 i=0; i<geom->num_normals; i++)
        simple.normals[hmget(normals, geom->normals[i])] = geom->normals[i];
    for (Uint32 i=0; i<geom->num_colors; i++)
        simple.colors[hmget(colors, geom->colors[i])] = geom->colors[i];
    for (Uint32 i=0; i<geom->num_faces; i++) {
        vec3ui verts_old = geom->faces[i].verts;
        vec3ui texs_old = geom->faces[i].texs;
        vec3ui norms_old = geom->faces[i].norms;
        vec3ui cols_old = geom->faces[i].cols;
        geom->faces[i].verts.x = hmget(vertices, geom->vertices[verts_old.x]);
        geom->faces[i].verts.y = hmget(vertices, geom->vertices[verts_old.y]);
        geom->faces[i].verts.z = hmget(vertices, geom->vertices[verts_old.z]);
        geom->faces[i].texs.x = hmget(textures, geom->textures[texs_old.x]);
        geom->faces[i].texs.y = hmget(textures, geom->textures[texs_old.y]);
        geom->faces[i].texs.z = hmget(textures, geom->textures[texs_old.z]);
        geom->faces[i].norms.x = hmget(normals, geom->normals[norms_old.x]);
        geom->faces[i].norms.y = hmget(normals, geom->normals[norms_old.y]);
        geom->faces[i].norms.z = hmget(normals, geom->normals[norms_old.z]);
        geom->faces[i].cols.x = hmget(colors, geom->colors[cols_old.x]);
        geom->faces[i].cols.y = hmget(colors, geom->colors[cols_old.y]);
        geom->faces[i].cols.z = hmget(colors, geom->colors[cols_old.z]);
    }
    hmfree(vertices);
    hmfree(textures);
    hmfree(normals);
    hmfree(colors);
    SDL_free(geom->vertices);
    SDL_free(geom->textures);
    SDL_free(geom->normals);
    SDL_free(geom->colors);
    geom->vertices = simple.vertices;
    geom->textures = simple.textures;
    geom->normals = simple.normals;
    geom->colors = simple.colors;
    geom->num_vertices = vertex_index;
    geom->num_textures = texture_index;
    geom->num_normals = normal_index;
    geom->num_colors = color_index;
    geom->vertices_size = vertex_index;
    geom->textures_size = texture_index;
    geom->normals_size = normal_index;
    geom->colors_size = color_index;
    return SDL_TRUE;
}

float _geom_error_from_lod(Uint32 lod) {
    // Gives the pixel error
    if (lod == 0)
        return 0.01f;
    else if (lod == 1)
        return 5.0f;
    else if (lod == 2)
        return 10.0f;
    else
        return 15.0f;
}

Uint32 _geom_get_vertices_from_radius(float radius, Uint32 lod) {
    // https://stackoverflow.com/questions/11774038/how-to-render-a-circle-with-as-few-vertices-as-possible
    // float error = _geom_error_from_lod(lod);
    // float th = SDL_acosf(2 * SDL_powf((1 - error / radius), 2) - 1);
    // Uint32 num_vertices = (Uint32) SDL_ceilf(2* (float)M_PI/th);
    // return num_vertices;
    lod += 1;
    radius += 1.0f;
    return 3;
}

Uint32 _geom_remaining_vertices(EsGeometry* geom) {
    return geom->vertices_size - geom->num_vertices;
}

Uint32 _geom_remaining_faces(EsGeometry* geom) {
    return geom->faces_size - geom->num_faces;
}

Uint32 _geom_remaining_textures(EsGeometry* geom) {
    return geom->textures_size - geom->num_textures;
}

Uint32 _geom_remaining_normals(EsGeometry* geom) {
    return geom->normals_size - geom->num_normals;
}

Uint32 _geom_remaining_colors(EsGeometry* geom) {
    return geom->colors_size - geom->num_colors;
}

EsGeometry geom_init_geometry_size(Uint32 vertices_size, Uint32 faces_size, Uint32 textures_size, Uint32 normals_size, Uint32 colors_size) {
    EsGeometry geom;
    geom.num_vertices = 0;
    geom.num_faces = 0;
    geom.num_textures = 0;
    geom.num_normals = 0;
    geom.num_colors = 0;
    geom.vertices_size = vertices_size;
    geom.faces_size = faces_size;
    geom.textures_size = textures_size;
    geom.normals_size = normals_size;
    geom.colors_size = colors_size;
    geom.vertices = (vec3*) SDL_malloc(geom.vertices_size * sizeof(vec3));
    geom.faces = (EsFace*) SDL_malloc(geom.faces_size * sizeof(EsFace));
    geom.textures = (vec2*) SDL_malloc(geom.textures_size * sizeof(vec2));
    geom.normals = (vec3*) SDL_malloc(geom.normals_size * sizeof(vec3));
    geom.colors = (vec3*) SDL_malloc(geom.colors_size * sizeof(vec3));
    return geom;
}

SDL_bool geom_add_vertices_memory(EsGeometry* geom, Uint32 vertices_size) {
    // TODO (07 Dec 2020 sam): Grow memory exponentially. Otherwise will need too many reallocs.
    vec3* new_vertices = (vec3*) SDL_realloc(geom->vertices, (geom->vertices_size+vertices_size) * sizeof(vec3));
    if (new_vertices == NULL)
        return SDL_FALSE;
    geom->vertices = new_vertices;
    geom->vertices_size += vertices_size;
    return SDL_TRUE;
}

SDL_bool geom_add_normals_memory(EsGeometry* geom, Uint32 normals_size) {
    vec3* new_normals = (vec3*) SDL_realloc(geom->normals, (geom->normals_size+normals_size) * sizeof(vec3));
    if (new_normals == NULL)
        return SDL_FALSE;
    geom->normals = new_normals;
    geom->normals_size += normals_size;
    return SDL_TRUE;
}

SDL_bool geom_add_colors_memory(EsGeometry* geom, Uint32 colors_size) {
    vec3* new_colors = (vec3*) SDL_realloc(geom->colors, (geom->colors_size+colors_size) * sizeof(vec3));
    if (new_colors == NULL)
        return SDL_FALSE;
    geom->colors = new_colors;
    geom->colors_size += colors_size;
    return SDL_TRUE;
}

SDL_bool geom_add_faces_memory(EsGeometry* geom, Uint32 faces_size) {
    EsFace* new_faces = (EsFace*) SDL_realloc(geom->faces, (geom->faces_size+faces_size) * sizeof(EsFace));
    if (new_faces == NULL)
        return SDL_FALSE;
    geom->faces = new_faces;
    geom->faces_size += faces_size;
    return SDL_TRUE;
}

SDL_bool geom_add_textures_memory(EsGeometry* geom, Uint32 textures_size) {
    vec2* new_textures = (vec2*) SDL_realloc(geom->textures, (geom->textures_size+textures_size) * sizeof(vec2));
    if (new_textures == NULL)
        return SDL_FALSE;
    geom->textures = new_textures;
    geom->textures_size += textures_size;
    return SDL_TRUE;
}

EsGeometry geom_init_geometry() {
    return geom_init_geometry_size(DEFAULT_NUM_VERTICES, DEFAULT_NUM_INDICES, DEFAULT_NUM_TEXTURES, DEFAULT_NUM_NORMALS, DEFAULT_NUM_COLORS);
}

void geom_destroy_geometry(EsGeometry* geom) {
    geom->num_vertices = 0;
    geom->num_faces = 0;
    geom->num_textures = 0;
    geom->num_normals = 0;
    geom->num_colors = 0;
    geom->vertices_size = 0;
    geom->faces_size = 0;
    geom->textures_size = 0;
    geom->normals_size = 0;
    geom->colors_size = 0;
    SDL_free(geom->vertices);
    SDL_free(geom->faces);
    SDL_free(geom->textures);
    SDL_free(geom->normals);
    SDL_free(geom->colors);
    return;
}

SDL_bool geom_add_cone(EsGeometry* geom, vec3 root, vec3 axis, float base_radius, float height, SDL_bool close, Uint32 lod) {
    Uint32 base_num_vertices = _geom_get_vertices_from_radius(base_radius, lod);
    Uint32 total_vertices = base_num_vertices + 1;
    if (close)
        total_vertices++;
    Uint32 total_faces = base_num_vertices;
    if (close)
        total_faces *= 2;
    if (_geom_remaining_vertices(geom) < total_vertices) {
        // We just add extra. Don't need to be exact.
        SDL_bool resize = geom_add_vertices_memory(geom, total_vertices);
        if (!resize) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Add cone : Could not alloc vertices\n");
            return SDL_FALSE;
        }
    }
    if (_geom_remaining_faces(geom) < total_faces) {
        SDL_bool resize = geom_add_faces_memory(geom, total_faces);
        if (!resize) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Add cone : Could not alloc faces\n");
            return SDL_FALSE;
        }
    }
    Uint32 first_vertex = geom->num_vertices;
    Uint32 first_face = geom->num_faces;
    geom->num_vertices += total_vertices;
    geom->num_faces += total_faces;
    vec3* vertices = (vec3*) SDL_malloc(total_vertices * sizeof(vec3));
    EsFace* faces = (EsFace*) SDL_malloc(total_faces * sizeof(EsFace));
    vertices[0] = build_vec3(0.0f, height, 0.0f);
    for (Uint32 i=1; i<base_num_vertices+1; i++) {
        float angle = (i-1.0f) / (base_num_vertices*1.0f) * (2.0f* (float)M_PI);
        float x = SDL_sinf(angle) * base_radius;
        float y = SDL_cosf(angle) * base_radius;
        vertices[i] = build_vec3(x, 0.0f, y);
    }
    if (close)
        vertices[base_num_vertices+1] = build_vec3(0.0f, 0.0f, 0.0f);
    // TODO (22 Nov 2020 sam): Align to root and axis
    vec3 y_axis = build_vec3(0.0f, 1.0f, 0.0f);
    axis = vec3_normalize(axis);
    vec3 perp_axis = vec3_cross(y_axis, axis);
    float angle = SDL_acosf(vec3_dot(y_axis, axis));
    for (Uint32 i=0; i<total_vertices; i++) {
        vertices[i] = vec3_add(root, rotate_about_origin_axis(vertices[i], angle, perp_axis));
    }
    for (Uint32 i=0; i<base_num_vertices-1; i++) {
        faces[i].verts = build_vec3ui(first_vertex+i+1, first_vertex+0, first_vertex+i+2);
    }
    faces[base_num_vertices-1].verts = build_vec3ui(first_vertex+(base_num_vertices-1)+1, first_vertex+0, first_vertex+1);
    if (close) {
        for (Uint32 i=0; i<base_num_vertices-1; i++) {
            faces[base_num_vertices+i].verts = build_vec3ui(first_vertex+i+2, first_vertex+base_num_vertices+1, first_vertex+i+1);
        }
        Uint32 last_face = (base_num_vertices*2) - 1;
        faces[last_face].verts = build_vec3ui(first_vertex+1, first_vertex+base_num_vertices+1, first_vertex+(base_num_vertices-1)+1);
    }
    SDL_memcpy(&geom->vertices[first_vertex], vertices, sizeof(vec3)*total_vertices);
    SDL_memcpy(&geom->faces[first_face], faces, sizeof(EsFace)*total_faces);
    SDL_free(vertices);
    SDL_free(faces);
    return SDL_TRUE;
}

SDL_bool geom_add_cs_surface(EsGeometry* geom, float base_radius, vec3 base_pos, vec3 base_axis, float tip_radius, vec3 tip_pos, vec3 tip_axis, vec2 tex, Uint32 lod, float tree_height, float branch_offset_start, float branch_offset_end) {
    Uint32 base_num_vertices = _geom_get_vertices_from_radius(base_radius, lod);
    Uint32 total_vertices = base_num_vertices * 2;
    Uint32 total_faces = base_num_vertices * 2;
    Uint32 total_textures = 1;
    Uint32 total_normals = total_vertices;
    Uint32 total_colors = total_vertices;
    if (_geom_remaining_vertices(geom) < total_vertices) {
        // We just add extra. Don't need to be exact.
        SDL_bool resize = geom_add_vertices_memory(geom, total_vertices);
        if (!resize) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Add cone : Could not alloc vertices\n");
            return SDL_FALSE;
        }
    }
    if (_geom_remaining_faces(geom) < total_faces) {
        SDL_bool resize = geom_add_faces_memory(geom, total_faces);
        if (!resize) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Add cone : Could not alloc faces\n");
            return SDL_FALSE;
        }
    }
    if (_geom_remaining_textures(geom) < total_textures) {
        SDL_bool resize = geom_add_textures_memory(geom, total_textures);
        if (!resize) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Add cone : Could not alloc textures\n");
            return SDL_FALSE;
        }
    }
    if (_geom_remaining_normals(geom) < total_normals) {
        SDL_bool resize = geom_add_normals_memory(geom, total_normals);
        if (!resize) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Add cone : Could not alloc normals\n");
            return SDL_FALSE;
        }
    }
    if (_geom_remaining_colors(geom) < total_colors) {
        SDL_bool resize = geom_add_colors_memory(geom, total_colors);
        if (!resize) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Add cone : Could not alloc colors\n");
            return SDL_FALSE;
        }
    }
    Uint32 first_vertex = geom->num_vertices;
    Uint32 first_face = geom->num_faces;
    Uint32 first_texture = geom->num_textures;
    Uint32 first_normal = geom->num_normals;
    Uint32 first_color = geom->num_colors;
    geom->num_vertices += total_vertices;
    geom->num_faces += total_faces;
    geom->num_textures += total_textures;
    geom->num_normals += total_normals;
    geom->num_colors += total_colors;
    vec3* vertices = (vec3*) SDL_malloc(total_vertices * sizeof(vec3));
    EsFace* faces = (EsFace*) SDL_malloc(total_faces * sizeof(EsFace));
    vec2* textures = (vec2*) SDL_malloc(total_textures * sizeof(vec2));
    vec3* normals = (vec3*) SDL_malloc(total_normals * sizeof(vec3));
    vec3* colors = (vec3*) SDL_malloc(total_colors * sizeof(vec3));
    if (vertices == NULL || faces == NULL || textures == NULL || normals == NULL || colors == NULL)
        SDL_Log("\n\n\nCOULD NOT ALLOC MEMORY\n\n\n");
    for (Uint32 i=0; i<base_num_vertices; i++) {
        float angle = (i-1.0f) / (base_num_vertices*1.0f) * (2.0f* (float)M_PI);
        float x = SDL_sinf(angle) * base_radius;
        float y = SDL_cosf(angle) * base_radius;
        vertices[i] = build_vec3(x, 0.0f, y);
        x = SDL_sinf(angle) * tip_radius;
        y = SDL_cosf(angle) * tip_radius;
        vertices[base_num_vertices+i] = build_vec3(x, 0.0f, y);
    }
    vec3 y_axis = build_vec3(0.0f, 1.0f, 0.0f);
    base_axis = vec3_normalize(base_axis);
    vec3 base_perp_axis = vec3_cross(y_axis, base_axis);
    float base_angle = SDL_acosf(vec3_dot(y_axis, base_axis));
    for (Uint32 i=0; i<base_num_vertices; i++) {
        vertices[i] = vec3_add(base_pos, rotate_about_origin_axis(vertices[i], base_angle, base_perp_axis));
        normals[i] = vec3_normalize(vec3_sub(vertices[i], base_pos));
        colors[i] = build_vec3(vertices[i].y/tree_height, branch_offset_start, 0.0f);
    }
    tip_axis = vec3_normalize(tip_axis);
    vec3 tip_perp_axis = vec3_cross(y_axis, tip_axis);
    float tip_angle = SDL_acosf(vec3_dot(y_axis, tip_axis));
    for (Uint32 i=0; i<base_num_vertices; i++) {
        vertices[i+base_num_vertices] = vec3_add(tip_pos, rotate_about_origin_axis(vertices[i+base_num_vertices], tip_angle, tip_perp_axis));
        normals[i+base_num_vertices] = vec3_normalize(vec3_sub(vertices[i+base_num_vertices], tip_pos));
        colors[i+base_num_vertices] = build_vec3(vertices[i+base_num_vertices].y/tree_height, branch_offset_end, 0.0f);
    }
    textures[0] = tex;
    for (Uint32 i=0; i<base_num_vertices-1; i++) {
        faces[2*i + 0].verts = build_vec3ui(first_vertex+i, first_vertex+i+1, first_vertex+base_num_vertices+i);
        faces[2*i + 1].verts = build_vec3ui(first_vertex+i+1, first_vertex+base_num_vertices+i+1, first_vertex+base_num_vertices+i);
        faces[2*i + 0].texs = build_vec3ui(first_texture, first_texture, first_texture);
        faces[2*i + 1].texs = build_vec3ui(first_texture, first_texture, first_texture);
        faces[2*i + 0].norms = build_vec3ui(first_normal+i, first_normal+i+1, first_normal+base_num_vertices+i);
        faces[2*i + 1].norms = build_vec3ui(first_normal+i+1, first_normal+base_num_vertices+i+1, first_normal+base_num_vertices+i);
        faces[2*i + 0].cols = build_vec3ui(first_color+i, first_color+i+1, first_color+base_num_vertices+i);
        faces[2*i + 1].cols = build_vec3ui(first_color+i+1, first_color+base_num_vertices+i+1, first_color+base_num_vertices+i);
    }
    Uint32 last_index = 2*(base_num_vertices-1);
    Uint32 last_vertex = base_num_vertices-1;
    Uint32 last_normal = base_num_vertices-1;
    Uint32 last_color = base_num_vertices-1;
    faces[last_index + 0].verts = build_vec3ui(first_vertex+last_vertex, first_vertex+0, first_vertex+base_num_vertices+last_vertex);
    faces[last_index + 1].verts = build_vec3ui(first_vertex+0, first_vertex+base_num_vertices+0, first_vertex+base_num_vertices+last_vertex);
    faces[last_index + 0].texs = build_vec3ui(first_texture, first_texture, first_texture);
    faces[last_index + 1].texs = build_vec3ui(first_texture, first_texture, first_texture);
    faces[last_index + 0].norms = build_vec3ui(first_normal+last_normal, first_normal+0, first_normal+base_num_vertices+last_normal);
    faces[last_index + 1].norms = build_vec3ui(first_normal+0, first_normal+base_num_vertices+0, first_normal+base_num_vertices+last_normal);
    faces[last_index + 0].cols = build_vec3ui(first_color+last_color, first_color+0, first_color+base_num_vertices+last_color);
    faces[last_index + 1].cols = build_vec3ui(first_color+0, first_color+base_num_vertices+0, first_color+base_num_vertices+last_color);
    SDL_memcpy(&geom->vertices[first_vertex], vertices, sizeof(vec3)*total_vertices);
    SDL_memcpy(&geom->faces[first_face], faces, sizeof(EsFace)*total_faces);
    SDL_memcpy(&geom->textures[first_texture], textures, sizeof(vec2)*total_textures);
    SDL_memcpy(&geom->normals[first_normal], normals, sizeof(vec3)*total_normals);
    SDL_memcpy(&geom->colors[first_color], colors, sizeof(vec3)*total_colors);
    SDL_free(vertices);
    SDL_free(faces);
    SDL_free(textures);
    SDL_free(normals);
    SDL_free(colors);
    return SDL_TRUE;
}

SDL_bool geom_add_oval(EsGeometry* geom, vec3 position, vec3 axis, vec3 normal, float length, float width, vec2 tex, Uint32 lod) {
    lod += 1;
    axis;
    Uint32 base_num_vertices = _geom_get_vertices_from_radius(width, lod);
    Uint32 total_vertices = base_num_vertices + 1;
    Uint32 total_faces = base_num_vertices;
    Uint32 total_textures = 1;
    Uint32 total_normals = 1;
    if (_geom_remaining_vertices(geom) < total_vertices) {
        // We just add extra. Don't need to be exact.
        SDL_bool resize = geom_add_vertices_memory(geom, total_vertices);
        if (!resize) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Add cone : Could not alloc vertices\n");
            return SDL_FALSE;
        }
    }
    if (_geom_remaining_faces(geom) < total_faces) {
        SDL_bool resize = geom_add_faces_memory(geom, total_faces);
        if (!resize) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Add cone : Could not alloc faces\n");
            return SDL_FALSE;
        }
    }
    if (_geom_remaining_textures(geom) < total_textures) {
        SDL_bool resize = geom_add_textures_memory(geom, total_textures);
        if (!resize) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Add cone : Could not alloc textures\n");
            return SDL_FALSE;
        }
    }
    if (_geom_remaining_normals(geom) < total_normals) {
        SDL_bool resize = geom_add_normals_memory(geom, total_normals);
        if (!resize) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Add cone : Could not alloc normals\n");
            return SDL_FALSE;
        }
    }
    Uint32 first_vertex = geom->num_vertices;
    Uint32 first_face = geom->num_faces;
    Uint32 first_texture = geom->num_textures;
    Uint32 first_normal = geom->num_normals;
    geom->num_vertices += total_vertices;
    geom->num_faces += total_faces;
    geom->num_textures += total_textures;
    geom->num_normals += total_normals;
    vec3* vertices = (vec3*) SDL_malloc(total_vertices * sizeof(vec3));
    EsFace* faces = (EsFace*) SDL_malloc(total_faces * sizeof(EsFace));
    vec2* textures = (vec2*) SDL_malloc(total_textures * sizeof(vec2));
    vec3* normals = (vec3*) SDL_malloc(total_normals * sizeof(vec3));
    if (vertices == NULL || faces == NULL || textures == NULL || normals == NULL)
        SDL_Log("\n\n\nCOULD NOT ALLOC MEMORY\n\n\n");
    for (Uint32 i=0; i<base_num_vertices; i++) {
        float angle = (i*1.0f) / (base_num_vertices*1.0f) * (2.0f* (float)M_PI);
        float x = SDL_sinf(angle) * width;
        float y = SDL_cosf(angle) * length;
        vertices[i] = build_vec3(x, 0.0f, y);
    }
    vertices[base_num_vertices] = build_vec3(0.0f, 0.0f, 0.0f);
    vec3 y_axis = build_vec3(0.0f, 1.0f, 0.0f);
    normal = vec3_normalize(normal);
    vec3 perp_axis = vec3_cross(y_axis, normal);
    float angle = SDL_acosf(vec3_dot(y_axis, normal));
    for (Uint32 i=0; i<total_vertices; i++) {
        vertices[i] = vec3_add(position, rotate_about_origin_axis(vertices[i], angle, perp_axis));
    }
    // TODO (26 Nov 2020 sam): align with axis as well.
    textures[0] = tex;
    normals[0] = perp_axis;
    for (Uint32 i=0; i<base_num_vertices-1; i++) {
        faces[i].verts = build_vec3ui(first_vertex+i, first_vertex+i+1, first_vertex+base_num_vertices);
        faces[i].texs = build_vec3ui(first_texture, first_texture, first_texture);
        faces[i].norms = build_vec3ui(first_normal, first_normal, first_normal);
    }
    faces[base_num_vertices-1].verts = build_vec3ui(first_vertex+base_num_vertices-1, first_vertex, first_vertex+base_num_vertices);
    faces[base_num_vertices-1].texs = build_vec3ui(first_texture, first_texture, first_texture);
    faces[base_num_vertices-1].norms = build_vec3ui(first_normal, first_normal, first_normal);
    SDL_memcpy(&geom->vertices[first_vertex], vertices, sizeof(vec3)*total_vertices);
    SDL_memcpy(&geom->faces[first_face], faces, sizeof(EsFace)*total_faces);
    SDL_memcpy(&geom->textures[first_texture], textures, sizeof(vec2)*total_textures);
    SDL_memcpy(&geom->normals[first_normal], normals, sizeof(vec3)*total_normals);
    SDL_free(vertices);
    SDL_free(faces);
    SDL_free(textures);
    SDL_free(normals);
    return SDL_TRUE;
}

SDL_bool geom_add_triple_quad_mesh(EsGeometry* geom, vec3 position, vec3 axis, float height, float width, vec2 tex1, vec2 tex2, Uint32 lod, float tree_height, float branch_length, vec3 branch_root_pos) {
    lod += 1;
    Uint32 total_vertices = 12;
    Uint32 total_faces = 6;  // TODO (04 Dec 2020 sam): Figure out whether we need to make this double faced
    Uint32 total_textures = 4;
    Uint32 total_normals = 3;
    Uint32 total_colors = total_vertices;
    if (_geom_remaining_vertices(geom) < total_vertices) {
        // We just add extra. Don't need to be exact.
        SDL_bool resize = geom_add_vertices_memory(geom, total_vertices);
        if (!resize) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Add cone : Could not alloc vertices\n");
            return SDL_FALSE;
        }
    }
    if (_geom_remaining_faces(geom) < total_faces) {
        SDL_bool resize = geom_add_faces_memory(geom, total_faces);
        if (!resize) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Add cone : Could not alloc faces\n");
            return SDL_FALSE;
        }
    }
    if (_geom_remaining_textures(geom) < total_textures) {
        SDL_bool resize = geom_add_textures_memory(geom, total_textures);
        if (!resize) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Add cone : Could not alloc textures\n");
            return SDL_FALSE;
        }
    }
    if (_geom_remaining_normals(geom) < total_normals) {
        SDL_bool resize = geom_add_normals_memory(geom, total_normals);
        if (!resize) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Add cone : Could not alloc normals\n");
            return SDL_FALSE;
        }
    }
    if (_geom_remaining_colors(geom) < total_colors) {
        SDL_bool resize = geom_add_colors_memory(geom, total_colors);
        if (!resize) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Add cone : Could not alloc colors\n");
            return SDL_FALSE;
        }
    }
    Uint32 first_vertex = geom->num_vertices;
    Uint32 first_face = geom->num_faces;
    Uint32 first_texture = geom->num_textures;
    Uint32 first_normal = geom->num_normals;
    Uint32 first_color = geom->num_colors;
    geom->num_vertices += total_vertices;
    geom->num_faces += total_faces;
    geom->num_textures += total_textures;
    geom->num_normals += total_normals;
    geom->num_colors += total_colors;
    vec3* vertices = (vec3*) SDL_malloc(total_vertices * sizeof(vec3));
    EsFace* faces = (EsFace*) SDL_malloc(total_faces * sizeof(EsFace));
    vec2* textures = (vec2*) SDL_malloc(total_textures * sizeof(vec2));
    vec3* normals = (vec3*) SDL_malloc(total_normals * sizeof(vec3));
    vec3* colors = (vec3*) SDL_malloc(total_colors * sizeof(vec3));
    if (vertices == NULL || faces == NULL || textures == NULL || normals == NULL || colors == NULL)
        SDL_Log("\n\n\nCOULD NOT ALLOC MEMORY\n\n\n");
    for (Uint32 i=0; i<6; i++) {
        vertices[i] = rotate_about_origin_yaxis(build_vec3(width/2.0f, 0.0f, 0.0f), i/6.0f * 2.0f * (float) M_PI);
        colors[i].z = 0.0f;
        vertices[i+6] = rotate_about_origin_yaxis(build_vec3(width/2.0f, height, 0.0f), i/6.0f * 2.0f * (float) M_PI);
        colors[i+6].z = 1.0f;
    }
    vec3 y_axis = build_vec3(0.0f, 1.0f, 0.0f);
    axis = vec3_normalize(axis);
    vec3 perp_axis = vec3_cross(y_axis, axis);
    float angle = SDL_acosf(vec3_dot(y_axis, axis));
    for (Uint32 i=0; i<total_vertices; i++) {
        vertices[i] = vec3_add(position, rotate_about_origin_axis(vertices[i], angle, perp_axis));
        colors[i].x = vertices[i].y / tree_height;
        colors[i].y = vec3_distance(vertices[i], branch_root_pos) / branch_length;
    }
    textures[0] = tex1;
    textures[1] = build_vec2(tex1.x, tex2.y);
    textures[2] = tex2;
    textures[3] = build_vec2(tex2.x, tex1.y);
    // TODO (04 Dec 2020 sam): Use proper normals here.
    normals[0] = vec3_normalize(axis);
    vec4ui* quads = (vec4ui*) SDL_malloc(3 * sizeof(vec4ui));
    quads[0] = build_vec4ui(first_vertex+0, first_vertex+3, first_vertex+9, first_vertex+6);
    quads[1] = build_vec4ui(first_vertex+1, first_vertex+4, first_vertex+10, first_vertex+7);
    quads[2] = build_vec4ui(first_vertex+2, first_vertex+5, first_vertex+11, first_vertex+8);
    vec4ui* cquads = (vec4ui*) SDL_malloc(3 * sizeof(vec4ui));
    cquads[0] = build_vec4ui(first_color+0, first_color+3, first_color+9, first_color+6);
    cquads[1] = build_vec4ui(first_color+1, first_color+4, first_color+10, first_color+7);
    cquads[2] = build_vec4ui(first_color+2, first_color+5, first_color+11, first_color+8);
    for (Uint32 i=0; i<total_faces/2; i++) {
        faces[i*2 + 0].verts = build_vec3ui(quads[i].x, quads[i].z, quads[i].y);
        faces[i*2 + 1].verts = build_vec3ui(quads[i].x, quads[i].w, quads[i].z);
        faces[i*2 + 0].texs = build_vec3ui(first_texture+0, first_texture+2, first_texture+1);
        faces[i*2 + 1].texs = build_vec3ui(first_texture+0, first_texture+3, first_texture+2);
        faces[i*2 + 0].cols = build_vec3ui(cquads[i].x, cquads[i].z, cquads[i].y);
        faces[i*2 + 1].cols = build_vec3ui(cquads[i].x, cquads[i].w, cquads[i].z);
    }
    for (Uint32 i=0; i<total_faces; i++) {
        faces[i].norms = build_vec3ui(first_normal, first_normal, first_normal);
    }
    SDL_memcpy(&geom->vertices[first_vertex], vertices, sizeof(vec3)*total_vertices);
    SDL_memcpy(&geom->faces[first_face], faces, sizeof(EsFace)*total_faces);
    SDL_memcpy(&geom->textures[first_texture], textures, sizeof(vec2)*total_textures);
    SDL_memcpy(&geom->normals[first_normal], normals, sizeof(vec3)*total_normals);
    SDL_memcpy(&geom->colors[first_color], colors, sizeof(vec3)*total_colors);
    SDL_free(vertices);
    SDL_free(faces);
    SDL_free(textures);
    SDL_free(normals);
    SDL_free(colors);
    SDL_free(quads);
    SDL_free(cquads);
    return SDL_TRUE;
}

SDL_bool geom_add_cone_origin_zaxis(EsGeometry* geom, float base_radius, float height, SDL_bool close, Uint32 lod) {
    const vec3 ORIGIN = build_vec3(0.0f, 0.0f, 0.0f);
    const vec3 ZAXIS = build_vec3(0.0f, 0.0f, 1.0f);
    return geom_add_cone(geom, ORIGIN, ZAXIS, base_radius, height, close, lod);
}

SDL_bool geom_save_obj(EsGeometry* geom, const char* filename) {
    SDL_RWops* obj_file;
    geom_simplify_geometry(geom);
    char* buffer = (char*) SDL_malloc(256 * sizeof(char));
    obj_file = SDL_RWFromFile(filename, "w");
    char* header = "o es_geom\n";
    SDL_RWwrite(obj_file, header, 1, SDL_strlen(header));
    for (Uint32 i=0; i<geom->num_vertices; i++) {
        vec3 vert = geom->vertices[i];
        SDL_snprintf(buffer, 256, "v %f %f %f\n", vert.x, vert.y, vert.z);
        SDL_RWwrite(obj_file, buffer, 1, SDL_strlen(buffer));
    }
    if (geom->num_textures == 0) {
        SDL_snprintf(buffer, 256, "vt 0.0 0.0\n");
        SDL_RWwrite(obj_file, buffer, 1, SDL_strlen(buffer));
    }
    for (Uint32 i=0; i<geom->num_textures; i++) {
        vec2 tex = geom->textures[i];
        SDL_snprintf(buffer, 256, "vt %f %f\n", tex.x, tex.y);
        SDL_RWwrite(obj_file, buffer, 1, SDL_strlen(buffer));
    }
    if (geom->num_normals == 0) {
        SDL_snprintf(buffer, 256, "vn 0.0 0.0 1.0\n");
        SDL_RWwrite(obj_file, buffer, 1, SDL_strlen(buffer));
    }
    for (Uint32 i=0; i<geom->num_normals; i++) {
        vec3 norm = geom->normals[i];
        SDL_snprintf(buffer, 256, "vn %f %f %f\n", norm.x, norm.y, norm.z);
        SDL_RWwrite(obj_file, buffer, 1, SDL_strlen(buffer));
    }
    for (Uint32 i=0; i<geom->num_faces; i++) {
        EsFace face = geom->faces[i];
        SDL_snprintf(buffer, 256, "f %i/%i/%i %i/%i/%i %i/%i/%i\n",
                face.verts.x+1, face.texs.x+1, face.norms.x+1,
                face.verts.y+1, face.texs.y+1, face.norms.y+1,
                face.verts.z+1, face.texs.z+1, face.norms.z+1);
        SDL_RWwrite(obj_file, buffer, 1, SDL_strlen(buffer));
    }
    SDL_RWclose(obj_file);
    SDL_free(buffer);
    return SDL_TRUE;
}
