#include "es_trees.h"
#include "es_geometrygen.h"

#include <stdlib.h>

#define TREES_DEFAULT_CS 128
#define TREES_DEFAULT_ROOTS 16

SDL_bool _trees_generate_branch(EsTree* tree, vec3 position, vec3 axis, Uint32 depth, float parent_length);
EsCrossSection _trees_build_cs(float radius, vec3 position, vec3 axis, Uint32 depth, Uint32 num_children, Uint32 child1);
float _get_var(EsVarFloat var);

EsCrossSection _trees_build_cs(float radius, vec3 position, vec3 axis, Uint32 depth, Uint32 num_children, Uint32 child1) {
    EsCrossSection cs;
    cs.radius = radius;
    cs.position = position;
    cs.axis = axis;
    cs.num_children = num_children;
    cs.children[0] = child1;
    cs.depth = depth;
    return cs;
}

float _get_var(EsVarFloat var) {
    rand();
    return var.val + (((float) rand() / (float) RAND_MAX) - 0.5f) * 2.0f * var.val_v;
}

// TODO (22 Nov 2020 sam): Implement get_next_cs_id or something. Function that gives the current cs_id
// and incrments num. Also checks size and reallocs if required.

SDL_bool _trees_generate_branch(EsTree* tree, vec3 position, vec3 axis, Uint32 depth, float parent_length) {
    float length;
    Uint32 index = tree->num_cross_sections;
    Uint32 num_segments = tree->params.curves_res[depth];
    Uint32 root_index = tree->num_roots;
    tree->num_cross_sections++;
    tree->num_roots++;
    tree->roots[root_index] = index;
    if (depth == 0)
        length = _get_var(tree->params.scale) * _get_var(tree->params.lengths[0]);
    float base_radius = length * tree->params.ratio * _get_var(tree->params.scale);
    SDL_Log("base_radius = %f\n", base_radius);
    vec3 current_pos = position;
    vec3 current_axis = axis;
    for (Uint32 i=0; i<num_segments+1; i++) {
        float current_radius = base_radius * lerp(base_radius, 1.0f - tree->params.tapers[depth], ((float)i/(float)num_segments));
        SDL_Log("current_radius = %f\n", current_radius);
        Uint32 num_children;
        Uint32 child_index;
        if (i+1 == num_segments+1) {
            num_children = 0;
            child_index = 0;
        } else { 
            num_children = 1;
            child_index = tree->num_cross_sections;
            tree->num_cross_sections++;
        }
        tree->cross_sections[index] = _trees_build_cs(current_radius, current_pos, current_axis, depth, num_children, child_index);
        current_pos = vec3_add(current_pos, vec3_scale(axis, length/tree->params.curves_res[depth]));
        index = child_index;
    }
    return SDL_TRUE;
}

EsTree trees_test() {
    EsTree tree;
    // Quaking Aspen params
    tree.params.lengths[0].val = 1;
    tree.params.lengths[0].val_v = 0;
    tree.params.scale.val = 13;
    tree.params.scale.val_v = 3;
    tree.params.ratio = 0.015f;
    tree.params.tapers[0] = 1.0f;
    tree.params.curves_res[0] = 3;
    tree.num_cross_sections = 0;
    tree.cross_sections_size = TREES_DEFAULT_CS;
    tree.cross_sections = (EsCrossSection*) SDL_malloc(tree.cross_sections_size * sizeof(EsCrossSection));
    tree.num_roots = 0;
    tree.roots_size = TREES_DEFAULT_ROOTS;
    tree.roots = (Uint32*) SDL_malloc(tree.roots_size * sizeof(Uint32));
    trees_generate(&tree);
    trees_to_obj(&tree, "tree.obj");
    return tree;
}



SDL_bool trees_generate(EsTree* tree) {
    _trees_generate_branch(tree, build_vec3(0.0f, 0.0f, 0.0f), build_vec3(0.0f, 1.0f, 0.0f), 0, 0.0);
    return SDL_TRUE;
}

SDL_bool trees_to_obj(EsTree* tree, const char* filename) {
    EsGeometry geom = geom_init_geometry();
    SDL_Log("%i %i\n", tree->num_cross_sections, tree->num_roots);
    for (Uint32 i=0; i<tree->num_roots; i++) {
        Uint32 current_cs = tree->roots[i];
        while (SDL_TRUE) {
            if (tree->cross_sections[current_cs].num_children == 0)
                break;
            EsCrossSection base = tree->cross_sections[current_cs];
            EsCrossSection tip = tree->cross_sections[base.children[0]];
            geom_add_cs_surface(&geom, base.radius, base.position, base.axis, tip.radius, tip.position, tip.axis, 0);
            current_cs = base.children[0];
            SDL_Log("%i -> radius = %f , pos = (%f, %f, %f) axis = (%f, %f, %f)\n", i, base.radius, base.position.x, base.position.y, base.position.z);
            SDL_Log("%i -> radius = %f , pos = (%f, %f, %f) axis = (%f, %f, %f)\n", i, tip.radius, tip.position.x, tip.position.y, tip.position.z);
        }
    }
    geom_save_obj(&geom, filename);
    return SDL_TRUE;
}
