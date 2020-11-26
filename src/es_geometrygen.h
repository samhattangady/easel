/*
 * es_geometrygen is a library to help create geometric shapes and output as obj
 */

#include "SDL.h"
#include "es_warehouse.h"
#define DEFAULT_NUM_VERTICES 1024
#define DEFAULT_NUM_INDICES 256

typedef struct {
    Uint32 num_vertices;
    Uint32 num_faces;
    Uint32 vertices_size;
    Uint32 faces_size;
    vec3* vertices;
    vec3ui* faces;
} EsGeometry;

extern EsGeometry geom_init_geometry();
extern EsGeometry geom_init_geometry_size(Uint32 vertices_size, Uint32 faces_size);
extern SDL_bool geom_add_vertices_memory(EsGeometry* geom, Uint32 vertices_size);
extern SDL_bool geom_add_faces_memory(EsGeometry* geom, Uint32 faces_size);
extern void geom_destroy_geometry(EsGeometry* geom);

extern SDL_bool geom_add_cone(EsGeometry* geom, vec3 root, vec3 axis, float base_radius, float height, SDL_bool close, Uint32 lod);
extern SDL_bool geom_add_cone_origin_zaxis(EsGeometry* geom, float base_radius, float height, SDL_bool close, Uint32 lod);
extern SDL_bool geom_add_cs_surface(EsGeometry* geom, float base_radius, vec3 base_pos, vec3 base_axis, float tip_radius, vec3 tip_pos, vec3 tip_axis, Uint32 lod);
extern SDL_bool geom_add_oval(EsGeometry* geom, vec3 position, vec3 axis, vec3 normal, float length, float width, Uint32 lod);

extern SDL_bool geom_save_obj(EsGeometry* geom, const char* filename);
