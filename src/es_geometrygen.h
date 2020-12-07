/*
 * es_geometrygen is a library to help create geometric shapes and output as obj
 */

#ifndef ES_GEOMETRYGEN_DEFINED
#define ES_GEOMETRYGEN_DEFINED


#include "SDL.h"
#include "es_warehouse.h"
#define DEFAULT_NUM_VERTICES 1024
#define DEFAULT_NUM_INDICES 256
#define DEFAULT_NUM_TEXTURES 128
#define DEFAULT_NUM_NORMALS 128
#define DEFAULT_NUM_COLORS 128

typedef struct {
    vec3ui verts;
    vec3ui texs;
    vec3ui norms;
    vec3ui cols;
} EsFace;

typedef struct {
    Uint32 num_vertices;
    Uint32 num_faces;
    Uint32 num_textures;
    Uint32 num_normals;
    Uint32 num_colors;
    Uint32 vertices_size;
    Uint32 faces_size;
    Uint32 textures_size;
    Uint32 normals_size;
    Uint32 colors_size;
    vec3* vertices;
    EsFace* faces;
    vec2* textures;
    vec3* normals;
    vec3* colors;
} EsGeometry;

extern EsGeometry geom_init_geometry();
extern EsGeometry geom_init_geometry_size(Uint32 vertices_size, Uint32 faces_size, Uint32 textures_size, Uint32 normals_size, Uint32 colors_size);
extern SDL_bool geom_add_vertices_memory(EsGeometry* geom, Uint32 vertices_size);
extern SDL_bool geom_add_faces_memory(EsGeometry* geom, Uint32 faces_size);
extern SDL_bool geom_add_textures_memory(EsGeometry* geom, Uint32 textures_size);
extern SDL_bool geom_add_normals_memory(EsGeometry* geom, Uint32 normals_size);
extern SDL_bool geom_add_colors_memory(EsGeometry* geom, Uint32 colors_size);
extern void geom_destroy_geometry(EsGeometry* geom);

extern SDL_bool geom_add_cone(EsGeometry* geom, vec3 root, vec3 axis, float base_radius, float height, SDL_bool close, Uint32 lod);
extern SDL_bool geom_add_cone_origin_zaxis(EsGeometry* geom, float base_radius, float height, SDL_bool close, Uint32 lod);
extern SDL_bool geom_add_cs_surface(EsGeometry* geom, float base_radius, vec3 base_pos, vec3 base_axis, float tip_radius, vec3 tip_pos, vec3 tip_axis, vec2 tex, Uint32 lod);
extern SDL_bool geom_add_oval(EsGeometry* geom, vec3 position, vec3 axis, vec3 normal, float length, float width, vec2 tex, Uint32 lod);
extern SDL_bool geom_add_triple_quad_mesh(EsGeometry* geom, vec3 position, vec3 axis, float height, float width, vec2 tex1, vec2 tex2, Uint32 lod);

extern SDL_bool geom_simplify_geometry(EsGeometry* geom);
extern SDL_bool geom_save_obj(EsGeometry* geom, const char* filename);

#endif
