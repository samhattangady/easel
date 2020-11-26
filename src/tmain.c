#include "SDL.h"
#include "es_warehouse.h"
#include "es_trees.h"

int main(int argc, char** argv) {
    argc; argv;
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
    SDL_Log("hello trees\n");
    trees_test("tree.obj");
    SDL_Log("created trees\n");
    return 0;
}
