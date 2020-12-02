/*
 * es_trees is a procedural tree generator based on the work of Penn and Webber
 * https://www2.cs.duke.edu/courses/cps124/spring08/assign/07_papers/p119-weber.pdf
 */

#ifndef ES_TREES_DEFINED
#define ES_TREES_DEFINED

#include "SDL.h"
#include "es_warehouse.h"
#include "es_geometrygen.h"

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
    EsVarFloat trunk_scale;
    float base_splits;
    EsVarFloat lengths[4];
    float tapers[4];
    Uint32 curves_res[4];
    EsVarFloat curves[4];
    float curves_back[4];
    EsVarFloat down_angles[4];
    EsVarFloat rotates[4];
    Uint32 branches[4];
    float seg_splits[4];
    EsVarFloat split_angles[4];
    float quality;
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
    vec3 position;
    vec3 axis;
    vec3 normal;
    float length;
    float width;
} EsLeaf;

typedef struct {
    Uint32 num_cross_sections;
    Uint32 cross_sections_size;
    EsCrossSection* cross_sections;
    Uint32 num_roots;
    Uint32 roots_size;
    Uint32* roots;
    Uint32 num_leaves;
    Uint32 leaves_size;
    EsLeaf* leaves;
    EsTreeParams params;
} EsTree;

extern EsTree trees_test(const char* objname);
extern EsTree trees_gen_test();
extern SDL_bool trees_generate(EsTree* tree);
extern EsGeometry trees_to_geom(EsTree* tree);
extern SDL_bool trees_to_obj(EsTree* tree, const char* filename);

#endif
