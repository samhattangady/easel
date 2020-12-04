#include "SDL.h"
#include "es_warehouse.h"
#include "es_geometrygen.h"

int main(int argc, char** argv) {
    argc; argv;
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
    EsGeometry cone = geom_init_geometry();
    geom_add_triple_quad_mesh(&cone, build_vec3(0.0f, 2.0f, 0.0f), build_vec3(1.0f, 1.0f, 1.0f), 2.5f, 2.5f, build_vec2(0.0f, 0.0f), 0);
    geom_save_obj(&cone, "cone.obj");
    return 0;
}
