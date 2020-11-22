/*
 * es_trees is a procedural tree generator based on the work of Penn and Webber
 * https://www2.cs.duke.edu/courses/cps124/spring08/assign/07_papers/p119-weber.pdf
 */

#include "SDL.h"
#include "es_warehouse.h"

typedef struct {
    float val;
    float val_v;
} EsVarFloat;

typedef struct {
    Uint32 shape;
    float base_size;
    EsVarFloat scale;
    Uint32 levels;
    float ratio;
    float ratio_power;
    Uint32 lobes;
    float lobes_depth;
    float flare;
    float trunk_scale;
    float trunk_scale_v;
    float base_splits;
    EsVarFloat lengths[4];
    float tapers[4];
    Uint32 curves_res[4];
    float curves[4];
    float curves_v[4];
    float curves_back[4];
    EsVarFloat down_angles[4];
    EsVarFloat rotates[4];
    Uint32 branches[4];
    Uint32 leaves;
    Uint32 leaf_shape;
    float leaf_scale;
    float leaf_scale_x;
    float attraction_up;
    float prune_ratio;
    float prune_width;
    float prune_width_peak;
    float prune_power_low;
    float prune_power_high;
} EsTreeParams;

typedef struct {
    float radius;
    vec3 position;
    vec3 axis;
    Uint32 num_children;
    Uint32 children[3];
    Uint32 depth;
} EsCrossSection;

typedef struct {
    Uint32 num_cross_sections;
    Uint32 cross_sections_size;
    EsCrossSection* cross_sections;
    Uint32 num_roots;
    Uint32 roots_size;
    Uint32* roots;
    EsTreeParams params;
} EsTree;

extern EsTree trees_test();
extern SDL_bool trees_generate(EsTree* tree);
extern SDL_bool trees_to_obj(EsTree* tree, const char* filename);
