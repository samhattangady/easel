#include "SDL.h"
#include "es_warehouse.h"
#include "es_geometrygen.h"

int main(int argc, char** argv) {
    argc; argv;
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
    EsGeometry cone = geom_init_geometry();
    geom_add_cone(&cone, build_vec3(0.0f, 0.0f, 0.0f), build_vec3(0.0f, 1.0f, 0.0f), 5.0f, 15.0f, SDL_TRUE, 0);
    geom_add_cone(&cone, build_vec3(0.0f, 2.0f, 0.0f), build_vec3(1.0f, 1.0f, 1.0f), 1.0f, 9.0f, SDL_FALSE, 0);
    geom_add_cone(&cone, build_vec3(0.0f, 5.0f, 0.0f), build_vec3(-1.0f, 1.0f, 1.0f), 0.4f, 6.0f, SDL_FALSE, 0);
    geom_save_obj(&cone, "cone.obj");
    return 0;
}
