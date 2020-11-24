#include "es_trees.h"
#include "es_geometrygen.h"

#include <stdlib.h>

#define TREES_DEFAULT_CS 2048
#define TREES_DEFAULT_ROOTS 128

SDL_bool _trees_generate_branch(EsTree* tree, vec3 position, vec3 axis, vec3 rotation_axis, Uint32 depth, float parent_length, float parent_radius, float offset);
EsCrossSection _trees_build_cs(float radius, vec3 position, vec3 axis, Uint32 depth, Uint32 num_children, Uint32 child1);
float _get_var(EsVarFloat var);
Uint32 _get_num_branches(EsTree* tree, Uint32 max_branches, float offset, float parent_length, float max_length, float child_length, Uint32 depth);
float _shape_ratio(EsTree* tree, float ratio);
float _get_branch_length(EsTree* tree, Uint32 depth, float parent_length, float offset);
float _get_branch_start_length(EsTree* tree, float length, Uint32 depth);
vec3 _lerp_branch(EsTree* tree, Uint32 root, float length);

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
    return var.val + (((float) rand() / (float) RAND_MAX) - 0.5f) * 2.0f * var.val_v;
}

float _shape_ratio(EsTree* tree, float ratio) {
    switch (tree->params.shape) {
        case 0:
            return 0.2f + (0.8f*ratio);
        case 7:
            if (ratio <= 0.7)
                return 0.5f + (0.5f * (ratio/0.7f));
            else
                return 0.5f + (0.5f * ((1.0f-ratio)/0.3f));
        default:
            return ratio;
    }
}

Uint32 _get_num_branches(EsTree* tree, Uint32 max_branches, float offset, float parent_length, float max_length, float child_length, Uint32 depth) {
    if (depth == 0)
        return max_branches;
    else if (depth == 1)
        return (Uint32) (max_branches * (0.2f + (0.8 * (child_length/parent_length) / max_length)));
    else
        return (Uint32) (max_branches * (1.0f - (0.5 * (offset/parent_length))));
}

float _get_branch_length(EsTree* tree, Uint32 depth, float parent_length, float offset) {
    if (depth == 0)
        return _get_var(tree->params.scale) * _get_var(tree->params.lengths[depth]);
    else if (depth == 1) {
        float base_length = tree->params.base_size * parent_length;
        float ratio = (parent_length-offset) / (parent_length-base_length);
        return parent_length * _get_var(tree->params.lengths[depth]) * _shape_ratio(tree, ratio);
    } else
        return _get_var(tree->params.lengths[depth]) * (parent_length - (0.6f*offset));
}

float _get_branch_start_length(EsTree* tree, float length, Uint32 depth) {
    if (depth == 0)
        return tree->params.base_size * length;
    else
        return 0.0f;
}

vec3 _branch_axis_rotation(EsTree* tree, vec3 current_axis, vec3 rotation_axis, Uint32 num_seg, Uint32 depth) {
    float angle = _get_var(tree->params.curves[depth]) / tree->params.curves_res[depth];
    angle = deg_to_rad(angle);
    return rotate_about_origin_axis(current_axis, angle, rotation_axis);
}

vec3 _lerp_branch(EsTree* tree, Uint32 root, float length) {
    EsCrossSection cs = tree->cross_sections[root];
    float rem = length;
    while (cs.num_children > 0) {
        // TODO (24 Nov 2020 sam): BranchSplitRefactor
        EsCrossSection child = tree->cross_sections[cs.children[0]];
        vec3 axis = vec3_sub(child.position, cs.position);
        float dist = vec3_magnitude(axis);
        if (rem < dist) {
            float offset = rem / dist;
            return vec3_add(cs.position, vec3_scale(axis, offset));
        }
        cs = child;
        rem -= dist;
    }
    SDL_Log("ERROR: Could not lerp branch: root=%i, length=%f\n", root, length);
    return build_vec3(10.0f, 10.0f, 10.0f);
}

// TODO (22 Nov 2020 sam): Implement get_next_cs_id or something. Function that gives the current cs_id
// and incrments num. Also checks size and reallocs if required.

SDL_bool _trees_generate_branch(EsTree* tree, vec3 position, vec3 axis, vec3 rotation_axis, Uint32 depth, float parent_length, float parent_radius, float offset) {
    Uint32 branch_root = tree->num_cross_sections;
    Uint32 num_segments = tree->params.curves_res[depth];
    Uint32 root_index = tree->num_roots;
    tree->num_cross_sections++;
    tree->num_roots++;
    tree->roots[root_index] = branch_root;
    Uint32 index = branch_root;
    float length = _get_branch_length(tree, depth, parent_length, offset);
    float base_radius;
    if (depth == 0)
        base_radius = length * tree->params.ratio * _get_var(tree->params.trunk_scale);
    else
        base_radius = parent_radius * SDL_powf(length / parent_length, tree->params.ratio_power);
    SDL_Log("radius is %f. pos = (%f, %f, %f)\n", base_radius, position.x, position.y, position.z);
    float tip_radius = base_radius * (1.0f-tree->params.tapers[depth]);
    vec3 current_pos = position;
    vec3 current_axis = axis;
    // create the branch
    for (Uint32 i=0; i<num_segments+1; i++) {
        float current_radius = lerp(base_radius, tip_radius, ((float)i/(float)num_segments));
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
        current_pos = vec3_add(current_pos, vec3_scale(current_axis, length/tree->params.curves_res[depth]));
        current_axis = _branch_axis_rotation(tree, current_axis, rotation_axis, i, depth);
        index = child_index;
    }
    // create the child branches
    // if (depth <= 2) {
    if (depth == 0) {  // nocheckin. testing one branch
        Uint32 max_branches = tree->params.branches[depth+1];
        // TODO (23 Nov 2020 sam): We should probably not be sending offset here. Need to check.
        float child_length = _get_branch_length(tree, depth+1, length, offset);
        Uint32 num_branches = _get_num_branches(tree, max_branches, offset, length, _get_var(tree->params.lengths[depth+1]), child_length, depth);
        float branch_start = _get_branch_start_length(tree, length, depth);
        float branch_end = length;
        SDL_Log("generating %i branches\n", num_branches);
        for (Uint32 i=0; i<num_branches; i++) {
            float child_offset_ratio = ((float) i + 0.5f) / (float) num_branches;
            float child_offset = lerp(branch_start, branch_end, child_offset_ratio);
            float radius_at_child = lerp(base_radius, tip_radius, child_offset_ratio);
            // TODO (23 Nov 2020 sam): Pos has to trace along the parent. Can't just lerp
            vec3 child_pos = _lerp_branch(tree, branch_root, lerp(branch_start, branch_end, child_offset_ratio));
            vec3 child_axis = build_vec3(0.0f, 0.0f, 1.0f);
            vec3 child_rotation_axis = build_vec3(1.0f, 0.0f, 0.0f);
            SDL_Log("generating branch num %i: %f\n", i, child_offset_ratio);
            _trees_generate_branch(tree, child_pos, child_axis, child_rotation_axis, depth+1, length, radius_at_child, child_offset);
        }
    }
    return SDL_TRUE;
}

EsTree trees_test() {
    EsTree tree;
    // Quaking Aspen params
    tree.params.shape = 7;
    tree.params.base_size = 0.4f;
    tree.params.scale.val = 13;
    tree.params.scale.val_v = 3;
    tree.params.levels = 3;
    tree.params.ratio = 0.015f;
    tree.params.ratio_power = 1.2f;
    tree.params.lobes = 5;
    tree.params.lobes_depth = 0.07f;
    tree.params.flare = 0.6f;
    tree.params.trunk_scale.val = 1.0f;
    tree.params.trunk_scale.val_v = 0.0f;
    tree.params.base_splits = 0;
    // level 0
    tree.params.lengths[0].val = 1;
    tree.params.lengths[0].val_v = 0;
    tree.params.tapers[0] = 1.0f;
    tree.params.curves_res[0] = 3;
    tree.params.curves[0].val = 0;
    tree.params.curves[0].val_v = 20;
    tree.params.curves_back[0] = 0;
    tree.params.down_angles[0].val = 0;
    tree.params.down_angles[0].val_v = 0;
    tree.params.rotates[0].val = 0;
    tree.params.rotates[0].val_v = 0;
    tree.params.branches[0] = 0;
    tree.params.seg_splits[0] = 0;
    tree.params.split_angles[0].val = 0.3f;
    tree.params.split_angles[0].val_v = 0;
    // level 1
    tree.params.lengths[1].val = 0.3f;
    tree.params.lengths[1].val_v = 0;
    tree.params.tapers[1] = 1.0f;
    tree.params.curves_res[1] = 5;
    tree.params.curves[1].val = -40;
    tree.params.curves[1].val_v = 50;
    tree.params.curves_back[1] = 0;
    tree.params.down_angles[1].val = 60;
    tree.params.down_angles[1].val_v = -50;
    tree.params.rotates[1].val = 140;
    tree.params.rotates[1].val_v = 0;
    tree.params.branches[1] = 50;
    tree.params.seg_splits[1] = 0;
    tree.params.split_angles[1].val = 0.3f;
    tree.params.split_angles[1].val_v = 0;
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
    _trees_generate_branch(tree, build_vec3(0.0f, 0.0f, 0.0f), build_vec3(0.0f, 1.0f, 0.0f), build_vec3(0.0f, 0.0f, 1.0f), 0, 0.0, 0.0, 0.0);
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
        }
    }
    geom_save_obj(&geom, filename);
    return SDL_TRUE;
}
