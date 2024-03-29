#include "es_trees.h"
#include "es_geometrygen.h"

// For rand and RAND_MAX
#include <stdlib.h>

#define TREES_DEFAULT_CS 2048
#define TREES_DEFAULT_ROOTS 128
#define TREES_DEFAULT_LEAVES 256
#define MINIMUM_DIST_RAYMARCH 0.01
#define MAXIMUM_DIST_RAYMARCH 100.0

SDL_bool _trees_generate_branch(EsTree* tree, vec3 position, vec3 axis, vec3 rotation_axis, Uint32 depth, float parent_length, float parent_radius, float offset);
EsCrossSection _trees_build_cs(float radius, vec3 position, vec3 axis, Uint32 depth, Uint32 num_children, Uint32 child1);
float _get_var(EsVarFloat var);
Uint32 _get_num_branches(Uint32 max_branches, float offset, float parent_length, float max_length, float child_length, Uint32 depth);
float _shape_ratio(Uint32 shape, float ratio);
float _get_branch_length(EsTree* tree, Uint32 depth, float parent_length, float offset);
float _get_branch_start_length(EsTree* tree, float length, Uint32 depth);
vec3 _lerp_branch(EsTree* tree, Uint32 root, float length);
Uint32 _get_segment_root(EsTree* tree, Uint32 root, float length);
vec3 _get_current_branch_axis(EsTree* tree, EsCrossSection cs);
float _get_down_angle(EsTree* tree, Uint32 depth, float parent_length, float offset);
SDL_bool _add_leaf_on_branch(EsTree* tree, EsBranchSDF* branch, Uint32 leaf_id, float leaf_width, float leaf_length);

float _get_time_in_seconds() {
    return (float) SDL_GetTicks()/1000.0f;
}

float _sdf_sphere(vec3 pos, vec3 center, float radius) {
    return vec3_distance(pos, center) - radius;
}

float _smin(float d1, float d2, float k) {
    float h = SDL_max(k-SDL_fabsf(d1-d2),0.0f);
    return SDL_min(d1, d2) - h*h*0.25f/k;
}

float _smax(float d1, float d2, float k) {
    float h = SDL_max(k-SDL_fabsf(d1-d2),0.0f);
    return SDL_max(d1, d2) + h*h*0.25f/k;
}

float _distance_field(vec3 pos, EsBranchSDF* branch) {
    float d = _sdf_sphere(pos, branch->main_pos, branch->main_radius);
    float d1 = _sdf_sphere(pos, branch->add1_pos, branch->add1_radius);
    d = _smin(d, d1, 1.0f);
    d1 = _sdf_sphere(pos, branch->sub_pos, branch->sub_radius);
    d = _smax(d, -d1, 0.3f);
    return d;
}

vec3 _raymarch(vec3 start, vec3 dir, EsBranchSDF* branch) {
    float d = 0.0f;
    vec3 p = start;
    for (Uint32 i=0; i<100; i++) {
        float dist = _distance_field(p, branch);
        p = vec3_add(p, vec3_scale(dir, dist));
        d += dist;
        if (dist < MINIMUM_DIST_RAYMARCH)
            return p;
        if (d > MAXIMUM_DIST_RAYMARCH) {
            SDL_Log("failed to find sdf. branch= (%f, %f)", branch->main_radius, branch->sub_radius);
            return build_vec3(0.0, 0.0, 0.0);
        }
    }
    SDL_Log("failed to find sdf. branch= (%f, %f)", branch->main_radius, branch->sub_radius);
    return build_vec3(0.0, 0.0, 0.0);
}

SDL_bool _add_leaf_on_branch(EsTree* tree, EsBranchSDF* branch, Uint32 leaf_id, float leaf_width, float leaf_length) {
   vec3 start = vec3_add(branch->main_pos, vec3_scale(rand_vec3(), branch->main_radius*5.0f));
   vec3 dir = vec3_normalize(vec3_sub(branch->main_pos, start));
   tree->leaves[leaf_id].position = _raymarch(start, dir, branch);
   tree->leaves[leaf_id].axis = rand_vec3();
   tree->leaves[leaf_id].length = leaf_length;
   tree->leaves[leaf_id].width = leaf_width;
   return SDL_TRUE;
}

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
    return var.val + rand_negpos() * var.val_v;
}

float _shape_ratio(Uint32 shape, float ratio) {
    switch (shape) {
        case 0:
            return 0.2f + (0.8f*ratio);
        case 4:
            return 0.5f + (0.5f*ratio);
        case 7:
            if (ratio <= 0.7)
                return 0.5f + (0.5f * (ratio/0.7f));
            else
                return 0.5f + (0.5f * ((1.0f-ratio)/0.3f));
        default:
            return ratio;
    }
}

Uint32 _get_num_branches(Uint32 max_branches, float offset, float parent_length, float max_length, float child_length, Uint32 depth) {
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
        return parent_length * _get_var(tree->params.lengths[depth]) * _shape_ratio(tree->params.shape, ratio);
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
    num_seg;
    float angle = _get_var(tree->params.curves[depth]) / tree->params.curves_res[depth];
    angle = deg_to_rad(angle);
    return rotate_about_origin_axis(current_axis, angle, rotation_axis);
}

vec3 _get_current_branch_axis(EsTree* tree, EsCrossSection cs) {
    EsCrossSection child = tree->cross_sections[cs.children[0]];
    vec3 axis = vec3_sub(child.position, cs.position);
    return axis;
}

vec3 _lerp_branch(EsTree* tree, Uint32 root, float length) {
    EsCrossSection cs = tree->cross_sections[root];
    float rem = length;
    while (cs.num_children > 0) {
        // TODO (24 Nov 2020 sam): BranchSplitRefactor
        EsCrossSection child = tree->cross_sections[cs.children[0]];
        vec3 axis = _get_current_branch_axis(tree, cs);
        float dist = vec3_distance(cs.position, child.position);
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

Uint32 _get_segment_root(EsTree* tree, Uint32 root, float length) {
    EsCrossSection cs = tree->cross_sections[root];
    float rem = length;
    Uint32 current_index = root;
    while (cs.num_children > 0) {
        // TODO (24 Nov 2020 sam): BranchSplitRefactor
        EsCrossSection child = tree->cross_sections[cs.children[0]];
        vec3 axis = _get_current_branch_axis(tree, cs);
        float dist = vec3_magnitude(axis);
        if (rem < dist) {
            return current_index;
        }
        current_index = cs.children[0];
        cs = child;
        rem -= dist;
    }
    SDL_Log("ERROR: Could not get segement branch: root=%i, length=%f\n", root, length);
    return 0;
}

float _get_down_angle(EsTree* tree, Uint32 depth, float parent_length, float offset) {
    float down_angle;
    EsVarFloat var = tree->params.down_angles[depth];
    if (var.val_v > 0)
        down_angle = -deg_to_rad(_get_var(tree->params.down_angles[depth]));
    else {
        float valv = rand_negpos() * var.val_v;
        float base_length = tree->params.base_size * parent_length;
        float ratio = (parent_length-offset) / (parent_length-base_length);
        down_angle = var.val + (valv * (1.0f - (2.0f * _shape_ratio(0, ratio))));
    }
    return deg_to_rad(down_angle);
}

// TODO (22 Nov 2020 sam): Implement get_next_cs_id or something. Function that gives the current cs_id
// and incrments num. Also checks size and reallocs if required.
Uint32 _get_next_cs_id(EsTree* tree) {
    Uint32 cs_id = tree->num_cross_sections;
    tree->num_cross_sections++;
    if (tree->num_cross_sections == tree->cross_sections_size) {
        Uint32 new_size = tree->cross_sections_size * 2;
        EsCrossSection* new_cs = (EsCrossSection*) SDL_realloc(tree->cross_sections, new_size*sizeof(EsCrossSection));
        if (new_cs == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not realloc crossections\n");
            exit(-2);
        }
        tree->cross_sections = new_cs;
        tree->cross_sections_size = new_size;
    }
    return cs_id;
}

Uint32 _get_next_root(EsTree* tree) {
    Uint32 root_id = tree->num_roots;
    tree->num_roots++;
    // SDL_Log("rootid = %i\n", root_id);
    if (tree->num_roots == tree->roots_size) {
        Uint32 new_size = tree->roots_size * 2;
        Uint32* new_r = (Uint32*) SDL_realloc(tree->roots, new_size*sizeof(Uint32));
        if (new_r == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not realloc roots\n");
            exit(-1);
        }
        tree->roots = new_r;
        tree->roots_size = new_size;
    }
    return root_id;
}

Uint32 _get_next_leaf(EsTree* tree) {
    Uint32 leaf_id = tree->num_leaves;
    tree->num_leaves++;
    // SDL_Log("rootid = %i\n", root_id);
    if (tree->num_leaves == tree->leaves_size) {
        Uint32 new_size = tree->leaves_size * 2;
        EsLeaf* new_r = (EsLeaf*) SDL_realloc(tree->leaves, new_size*sizeof(EsLeaf));
        if (new_r == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not realloc leaves\n");
            exit(-1);
        }
        tree->leaves = new_r;
        tree->leaves_size = new_size;
    }
    return leaf_id;
}

Uint32 _get_next_branch(EsTree* tree) {
    Uint32 sdf_id = tree->num_sdfs;
    tree->num_sdfs++;
    // SDL_Log("rootid = %i\n", root_id);
    if (tree->num_sdfs == tree->sdfs_size) {
        Uint32 new_size = tree->sdfs_size * 2;
        EsBranchSDF* new_r = (EsBranchSDF*) SDL_realloc(tree->sdfs, new_size*sizeof(EsBranchSDF));
        if (new_r == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not realloc sdfs\n");
            exit(-1);
        }
        tree->sdfs = new_r;
        tree->sdfs_size = new_size;
    }
    return sdf_id;
}

SDL_bool _trees_generate_branch(EsTree* tree, vec3 position, vec3 axis, vec3 rotation_axis, Uint32 depth, float parent_length, float parent_radius, float offset) {
    Uint32 branch_root = _get_next_cs_id(tree);
    Uint32 root_index = _get_next_root(tree);
    if (depth == 0)
        tree->tree_root = root_index;
    Uint32 num_segments = tree->params.curves_res[depth];
    tree->roots[root_index] = branch_root;
    Uint32 index = branch_root;
    float length = _get_branch_length(tree, depth, parent_length, offset);
    if (depth == 0)
        tree->tree_height = length;
    float base_radius;
    if (depth == 0)
        base_radius = length * tree->params.ratio * _get_var(tree->params.trunk_scale);
    else
        base_radius = parent_radius * SDL_powf(length / parent_length, tree->params.ratio_power);
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
            child_index = _get_next_cs_id(tree);
        }
        tree->cross_sections[index] = _trees_build_cs(current_radius, current_pos, current_axis, depth, num_children, child_index);
        current_pos = vec3_add(current_pos, vec3_scale(current_axis, length/tree->params.curves_res[depth]));
        current_axis = _branch_axis_rotation(tree, current_axis, rotation_axis, i, depth);
        index = child_index;
    }
    // create the child branches
    Uint32 param_ref_depth = SDL_min(depth, 3);
    if (param_ref_depth < SDL_min(tree->params.levels-1, 3)) {
        Uint32 max_branches = tree->params.branches[param_ref_depth+1];
        // TODO (23 Nov 2020 sam): We should probably not be sending offset here. Need to check.
        float child_length = _get_branch_length(tree, param_ref_depth+1, length, 0.0f);
        float max_length = _get_var(tree->params.lengths[param_ref_depth+1]);
        Uint32 num_branches = _get_num_branches(max_branches, offset, parent_length, max_length, child_length, param_ref_depth);
        float branch_start = _get_branch_start_length(tree, length, param_ref_depth);
        float branch_end = length;
        vec3 current_rotation = build_vec3(0.0f, 0.0f, 1.0f);
        for (Uint32 i=0; i<num_branches; i++) {
            float child_offset_ratio = ((float) i + 0.5f) / (float) num_branches;
            float child_offset = lerp(branch_start, branch_end, child_offset_ratio);
            vec3 child_pos = _lerp_branch(tree, branch_root, child_offset);
            vec3 current_branch_axis = _get_current_branch_axis(tree, tree->cross_sections[branch_root]);
            float angle = deg_to_rad(_get_var(tree->params.rotates[param_ref_depth+1]));
            current_rotation = rotate_about_origin_axis(current_rotation, angle, current_branch_axis);
            vec3 child_rotation_axis = vec3_cross(current_rotation, current_branch_axis);
            float down_angle = _get_down_angle(tree, param_ref_depth+1, length, child_offset);
            vec3 child_axis = rotate_about_origin_axis(current_rotation, down_angle, child_rotation_axis);
            _trees_generate_branch(tree, child_pos, child_axis, child_rotation_axis, depth+1, length, base_radius, child_offset);
        }
    }
    if (depth == tree->params.levels-1) {
        // generate leaves
        Uint32 num_leaves = (Uint32) (tree->params.leaves * _shape_ratio(4, (offset/parent_length)));
        float tree_length = tree->tree_height;
        EsBranchSDF branch;
        branch.main_pos = _lerp_branch(tree, branch_root, 0.5f * length);
        branch.main_radius = length*0.5f + rand_negpos() * 0.1f;
        branch.add1_pos = vec3_add(branch.main_pos, vec3_scale(rand_vec3(), branch.main_radius+rand_pos()));
        branch.add1_radius = rand_pos() * branch.main_radius * 0.5f;
        branch.sub_pos = _lerp_branch(tree, tree->tree_root, 0.5f * tree_length);
        branch.sub_radius = vec3_distance(branch.sub_pos, branch.main_pos) - (branch.main_radius*2.0f);
        branch.sub_radius -= 0.2f * branch.sub_radius * rand_pos();
        Uint32 branch_id = _get_next_branch(tree);
        tree->sdfs[branch_id] = branch;
        float leaf_length = tree->params.leaf_scale / SDL_sqrtf(tree->params.quality);
        float leaf_width = tree->params.leaf_scale * tree->params.leaf_scale_x / SDL_sqrtf(tree->params.quality);
        for (Uint32 i=0; i<num_leaves; i++) {
            Uint32 leaf_id = _get_next_leaf(tree);
            _add_leaf_on_branch(tree, &branch, leaf_id, leaf_width, leaf_length);
            tree->leaves[leaf_id].branch_root = root_index;
            tree->leaves[leaf_id].sdf_id = branch_id;
        }
    }
    return SDL_TRUE;
}

extern EsTree trees_gen_test() {
    EsTree tree;
    // Quaking Aspen params
    tree.params.quality = 1.0f;
    tree.params.shape = 7;
    tree.params.base_size = 0.4f;
    tree.params.scale.val = 13;
    tree.params.scale.val_v = 3;
    tree.params.levels = 2;
    tree.params.ratio = 0.015f;
    tree.params.ratio_power = 1.0f;
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
    tree.params.split_angles[0].val = 0;
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
    tree.params.split_angles[1].val = 0;
    tree.params.split_angles[1].val_v = 0;
    // level 2
    tree.params.lengths[2].val = 0.6f;
    tree.params.lengths[2].val_v = 0;
    tree.params.tapers[2] = 1.0f;
    tree.params.curves_res[2] = 3;
    tree.params.curves[2].val = -40;
    tree.params.curves[2].val_v = 75;
    tree.params.curves_back[2] = 0;
    tree.params.down_angles[2].val = 45;
    tree.params.down_angles[2].val_v = 10;
    tree.params.rotates[2].val = 140;
    tree.params.rotates[2].val_v = 0;
    tree.params.branches[2] = 30;
    tree.params.seg_splits[2] = 0;
    tree.params.split_angles[2].val = 0;
    tree.params.split_angles[2].val_v = 0;
    // level 3
    tree.params.lengths[3].val = 0;
    tree.params.lengths[3].val_v = 0;
    tree.params.tapers[3] = 1.0f;
    tree.params.curves_res[3] = 1;
    tree.params.curves[3].val = 0;
    tree.params.curves[3].val_v = 0;
    tree.params.curves_back[3] = 0;
    tree.params.down_angles[3].val = 45;
    tree.params.down_angles[3].val_v = 10;
    tree.params.rotates[3].val = 77;
    tree.params.rotates[3].val_v = 0;
    tree.params.branches[3] = 1;
    tree.params.seg_splits[3] = 0;
    tree.params.split_angles[3].val = 0.3f;
    tree.params.split_angles[3].val_v = 0;
    tree.params.leaves = 35;
    tree.params.leaf_shape = 0;
    tree.params.leaf_scale = 0.57f;
    tree.params.leaf_scale_x = 1.0f;
    tree.params.attraction_up = 0.5;
    tree.params.prune_ratio = 0;
    tree.params.prune_width = 0.5;
    tree.params.prune_width_peak = 0.5;
    tree.params.prune_power_low = 0.5;
    tree.params.prune_power_high = 0.5;
    tree.num_cross_sections = 0;
    tree.cross_sections_size = TREES_DEFAULT_CS;
    tree.cross_sections = (EsCrossSection*) SDL_malloc(tree.cross_sections_size * sizeof(EsCrossSection));
    tree.num_roots = 0;
    tree.roots_size = TREES_DEFAULT_ROOTS;
    tree.roots = (Uint32*) SDL_malloc(tree.roots_size * sizeof(Uint32));
    tree.num_leaves = 0;
    tree.leaves_size = TREES_DEFAULT_LEAVES;
    tree.leaves = (EsLeaf*) SDL_malloc(tree.leaves_size * sizeof(EsLeaf));
    tree.num_sdfs = 0;
    tree.sdfs_size = TREES_DEFAULT_ROOTS;
    tree.sdfs = (EsBranchSDF*) SDL_malloc(tree.sdfs_size * sizeof(EsBranchSDF));
    
    trees_generate(&tree);
    return tree;
}

EsTree trees_test(const char* objname) {
    EsTree tree = trees_gen_test();
    trees_to_obj(&tree, objname);
    return tree;
}

SDL_bool trees_generate(EsTree* tree) {
    _trees_generate_branch(tree, build_vec3(0.0f, 0.0f, 0.0f), build_vec3(0.0f, 1.0f, 0.0f), build_vec3(0.0f, 0.0f, 1.0f), 0, 0.0, 0.0, 0.0);
    return SDL_TRUE;
}

SDL_bool trees_add_to_geom_at_pos(EsTree* tree, EsGeometry* geom, vec3 pos) {
    float* branch_lengths = SDL_malloc(tree->num_roots * sizeof(float));
    for (Uint32 i=0; i<tree->num_roots; i++) {
        Uint32 current_cs = tree->roots[i];
        float branch_length;
        float branch_offset_start;
        float branch_offset_end;
        branch_offset_start = 0.0f;
        branch_offset_end = 0.0f;
        if (i != tree->tree_root) {
            branch_length = 0.0f;
            SDL_bool run_loop = SDL_TRUE;
            while (run_loop) {
                if (tree->cross_sections[current_cs].num_children == 0)
                    break;
                EsCrossSection base = tree->cross_sections[current_cs];
                EsCrossSection tip = tree->cross_sections[base.children[0]];
                branch_length += vec3_distance(base.position, tip.position);
                current_cs = base.children[0];
            }
            branch_lengths[i] = branch_length;
            current_cs = tree->roots[i];
        } else {
            branch_length = tree->tree_height;
            branch_lengths[i] = branch_length;
        }
        SDL_bool run_loop = SDL_TRUE;
        while (run_loop) {
            if (tree->cross_sections[current_cs].num_children == 0)
                break;
            EsCrossSection base = tree->cross_sections[current_cs];
            EsCrossSection tip = tree->cross_sections[base.children[0]];
            if (i != tree->tree_root)
                branch_offset_end = branch_offset_start + vec3_distance(base.position, tip.position)/branch_length; 
            geom_add_cs_surface(geom, base.radius, vec3_add(pos, base.position), base.axis, tip.radius, vec3_add(pos, tip.position), tip.axis, build_vec2(0.0, 0.0), 0, tree->tree_height, branch_offset_start, branch_offset_end);
            current_cs = base.children[0];
            if (i != tree->tree_root)
                branch_offset_start = branch_offset_end;
        }
    }
    for (Uint32 i=0; i<tree->num_leaves; i++) {
        EsLeaf leaf = tree->leaves[i];
        if (vec3_is_zero(leaf.position))
            continue;
        geom_add_triple_quad_mesh(geom, vec3_add(pos, leaf.position), leaf.axis, leaf.length, leaf.width, build_vec2(0.03f, 0.03f), build_vec2(1.0f, 1.0f), 0, tree->tree_height, vec3_distance(tree->cross_sections[leaf.branch_root].position, tree->sdfs[leaf.sdf_id].main_pos), vec3_add(pos, tree->cross_sections[leaf.branch_root].position));
    }
    return SDL_TRUE;    
}

SDL_bool trees_add_to_geom(EsTree* tree, EsGeometry* geom) {
    return trees_add_to_geom_at_pos(tree, geom, build_vec3(0, 0, 0));
}

EsGeometry trees_to_geom(EsTree* tree) {
    EsGeometry geom = geom_init_geometry();
    trees_add_to_geom(tree, &geom);
    return geom;
}

SDL_bool trees_to_obj(EsTree* tree, const char* filename) {
    float time;
    time = _get_time_in_seconds();
    EsGeometry geom = trees_to_geom(tree);
    SDL_Log("meshing took %f seconds\n", _get_time_in_seconds()-time);
    time = _get_time_in_seconds();
    geom_save_obj(&geom, filename);
    SDL_Log("filesave took %f seconds\n", _get_time_in_seconds()-time);
    return SDL_TRUE;
}
