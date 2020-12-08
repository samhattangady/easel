#include "es_painter.h"
#include "SDL_vulkan.h"
#include "es_geometrygen.h"
#include "es_trees.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"
#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader_c.h"

#include <stdlib.h>

#define GRASS_INSTANCES 2002
#define GRASS_NUM_VERTICES 4
#define GRASS_MODEL_PATH "data/obj/grass3.obj"
#define GRASS_MODEL_TEXTURE_PATH "data/img/grass4.png"
#define GRASS_RADIUS 10.0f
#define SKYBOX_MODEL_PATH "data/obj/skybox.obj"
#define SKYBOX_MODEL_TEXTURE_PATH4 "data/img/skybox/front.jpg"
#define SKYBOX_MODEL_TEXTURE_PATH5 "data/img/skybox/back.jpg"
#define SKYBOX_MODEL_TEXTURE_PATH2 "data/img/skybox/top.jpg"
#define SKYBOX_MODEL_TEXTURE_PATH3 "data/img/skybox/bottom.jpg"
#define SKYBOX_MODEL_TEXTURE_PATH1 "data/img/skybox/left.jpg"
#define SKYBOX_MODEL_TEXTURE_PATH0 "data/img/skybox/right.jpg"
#define TREE_INSTANCES 1
#define TREE_MODEL_TEXTURE_PATH "data/img/tree.png"
#define GROUND_MODEL_TEXTURE_PATH "data/img/ground.png"
#define GROUND_NUM_VERTICES_SIDE 30

#include "es_painter_helpers.h"

SDL_bool _painter_create_swapchain(EsPainter* painter);
SDL_bool _painter_load_data(EsPainter* painter);

SDL_bool _painter_load_data(EsPainter* painter) {
    tinyobj_attrib_t attrib;
    tinyobj_shape_t* shapes = NULL;
    size_t num_shapes;
    tinyobj_material_t* materials = NULL;
    size_t num_materials;
    Uint32 flags = TINYOBJ_FLAG_TRIANGULATE;
    int ret;

    ShaderData tree_shader = painter->shaders[0];
    ShaderData grass_shader = painter->shaders[1];
    ShaderData ground_shader = painter->shaders[2];

    grass_shader.shader_name = "Grass Shader";
    grass_shader.vertex_shader = "data/spirv/grass_vertex.spv";
    grass_shader.fragment_shader = "data/spirv/fragment.spv";
    grass_shader.texture_filepath = GRASS_MODEL_TEXTURE_PATH;
    tree_shader.shader_name = "Tree Shader";
    tree_shader.vertex_shader = "data/spirv/tree_vertex.spv";
    tree_shader.fragment_shader = "data/spirv/tree_fragment.spv";
    tree_shader.texture_filepath = TREE_MODEL_TEXTURE_PATH;
    ground_shader.shader_name = "Ground Shader";
    ground_shader.vertex_shader = "data/spirv/vertex.spv";
    ground_shader.fragment_shader = "data/spirv/fragment.spv";
    ground_shader.texture_filepath = GROUND_MODEL_TEXTURE_PATH;
    painter->skybox_shader->shader_name = "Skybox Shader";
    painter->skybox_shader->vertex_shader = "data/spirv/skybox_vertex.spv";
    painter->skybox_shader->fragment_shader = "data/spirv/skybox_fragment.spv";

    ret = tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials,
                            &num_materials, GRASS_MODEL_PATH, _painter_read_obj_file, flags);
    if (ret != TINYOBJ_SUCCESS) {
        warehouse_error_popup("Error in Setup.", "Could not load model");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    grass_shader.num_vertices = attrib.num_vertices * GRASS_INSTANCES;
    grass_shader.vertices = (EsVertex*) SDL_malloc(grass_shader.num_vertices * sizeof(EsVertex));
    grass_shader.num_indices = attrib.num_faces * GRASS_INSTANCES;
    grass_shader.indices = (Uint32*) SDL_malloc(grass_shader.num_indices * sizeof(Uint32));
    if (num_shapes != 1) {
        warehouse_error_popup("Error in Setup.", "Currently only support obj with 1 shape");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    for (Uint32 i=0; i<attrib.num_vertices; i++) {
        EsVertex vert;
        vert.pos.x = attrib.vertices[i*3 + 0];
        vert.pos.y = attrib.vertices[i*3 + 1];
        vert.pos.z = attrib.vertices[i*3 + 2];
        // We're loading color field with same data for ease of billboarding
        vert.color.x = attrib.vertices[i*3 + 0];
        vert.color.y = attrib.vertices[i*3 + 1];
        vert.color.z = attrib.vertices[i*3 + 2];
        // TODO (28 Nov 2020 sam): Load this from the tinyobj parse
        vert.normal.x = 0.0f;
        vert.normal.y = 0.0f;
        vert.normal.z = 1.0f;
        grass_shader.vertices[i] = vert;
    }
    for (Uint32 i=0; i<attrib.num_faces; i++) {
        tinyobj_vertex_index_t face = attrib.faces[i];
        grass_shader.indices[i] = face.v_idx;
        grass_shader.vertices[face.v_idx].tex.x = attrib.texcoords[face.vt_idx*2 + 0];
        grass_shader.vertices[face.v_idx].tex.y = attrib.texcoords[face.vt_idx*2 + 1];
    }
    for (Uint32 i=1; i<GRASS_INSTANCES; i++) {
        float x = rand_negpos() * GRASS_RADIUS;
        float z = rand_negpos() * GRASS_RADIUS;
        if (vec3_magnitude(build_vec3(x,0,z)) > GRASS_RADIUS) {
            i--;
            continue;
        }
        float y = 1.0f * stb_perlin_noise3(x/10.0f, 0, z/10.0f, 0, 0, 0);
        for (Uint32 j=0; j<GRASS_NUM_VERTICES; j++) {
            EsVertex vert = grass_shader.vertices[j];
            vert.pos.x += x;
            vert.pos.y += y;
            vert.pos.z += z;        
            grass_shader.vertices[i*GRASS_NUM_VERTICES + j] = vert;
        }
        for (Uint32 j=0; j<attrib.num_faces; j++) {
            grass_shader.indices[i*attrib.num_faces + j] = i*GRASS_NUM_VERTICES + grass_shader.indices[j];
        }
    }
    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, num_shapes);
    tinyobj_materials_free(materials, num_materials);

    ret = tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials,
                            &num_materials, SKYBOX_MODEL_PATH, _painter_read_obj_file, flags);
    if (ret != TINYOBJ_SUCCESS) {
        warehouse_error_popup("Error in Setup.", "Could not load model");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    painter->skybox_shader->num_vertices = attrib.num_vertices;
    painter->skybox_shader->vertices = (EsVertex*) SDL_malloc(painter->skybox_shader->num_vertices * sizeof(EsVertex));
    painter->skybox_shader->num_indices = attrib.num_faces;
    painter->skybox_shader->indices = (Uint32*) SDL_malloc(painter->skybox_shader->num_indices * sizeof(Uint32));
    if (num_shapes != 1) {
        warehouse_error_popup("Error in Setup.", "Currently only support obj with 1 shape");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    for (Uint32 i=0; i<attrib.num_vertices; i++) {
        EsVertex vert;
        vert.pos.x = attrib.vertices[i*3 + 0];
        vert.pos.y = attrib.vertices[i*3 + 1];
        vert.pos.z = attrib.vertices[i*3 + 2];
        vert.color.x = 0.0f;
        vert.color.y = 0.0f;
        vert.color.z = 0.0f;
        painter->skybox_shader->vertices[i] = vert;
    }
    for (Uint32 i=0; i<attrib.num_faces; i++) {
        tinyobj_vertex_index_t face = attrib.faces[i];
        painter->skybox_shader->indices[i] = face.v_idx;
        // painter->skybox_shader->vertices[face.v_idx].tex.x = attrib.texcoords[face.vt_idx*2 + 0];
        // painter->skybox_shader->vertices[face.v_idx].tex.y = 1.0f - attrib.texcoords[face.vt_idx*2 + 1];
    }

    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, num_shapes);
    tinyobj_materials_free(materials, num_materials);

    EsGeometry tree = geom_init_geometry_size(300000, 700000, 2, 300000, 0);
    for (Uint32 i=0; i<TREE_INSTANCES; i++) {
        float x = rand_negpos() * GRASS_RADIUS;
        float z = rand_negpos() * GRASS_RADIUS;
        if (i == 0) {
            x = 0.0f;
            z = 0.0f;
        }
        if (vec3_magnitude(build_vec3(x,0,z)) > GRASS_RADIUS/2.0f) {
            i--;
            continue;
        }
        float y = 1.0f * stb_perlin_noise3(x/10.0f, 0, z/10.0f, 0, 0, 0);
        vec3 pos = build_vec3(x, y, z);
        EsTree tree_raw = trees_gen_test();
        trees_add_to_geom_at_pos(&tree_raw, &tree, pos);
    }
    geom_simplify_geometry(&tree);
    tree_shader.num_vertices = tree.num_vertices;
    tree_shader.vertices = (EsVertex*) SDL_malloc(tree_shader.num_vertices * sizeof(EsVertex));
    tree_shader.num_indices = tree.num_faces * 3;
    tree_shader.indices = (Uint32*) SDL_malloc(tree_shader.num_indices * sizeof(Uint32));
    for (Uint32 i=0; i<tree.num_vertices; i++) {
        EsVertex vert;
        vert.pos = tree.vertices[i];
        vert.color.x = 0.0;
        vert.color.y = 0.0;
        vert.color.z = 0.0;
        tree_shader.vertices[i] = vert;
    }
    for (Uint32 i=0; i<tree.num_faces; i++) {
        EsFace face = tree.faces[i];
        EsVertex vert_x = tree_shader.vertices[face.verts.x];
        EsVertex vert_y = tree_shader.vertices[face.verts.y];
        EsVertex vert_z = tree_shader.vertices[face.verts.z];
        vert_x.tex = tree.textures[face.texs.x];
        vert_y.tex = tree.textures[face.texs.y];
        vert_z.tex = tree.textures[face.texs.z];
        vert_x.normal = tree.normals[face.norms.x];
        vert_y.normal = tree.normals[face.norms.y];
        vert_z.normal = tree.normals[face.norms.z];
        vert_x.color = tree.colors[face.cols.x];
        vert_y.color = tree.colors[face.cols.y];
        vert_z.color = tree.colors[face.cols.z];
        tree_shader.vertices[face.verts.x] = vert_x;
        tree_shader.vertices[face.verts.y] = vert_y;
        tree_shader.vertices[face.verts.z] = vert_z;
        tree_shader.indices[i*3 + 0] = face.verts.x;
        tree_shader.indices[i*3 + 1] = face.verts.y;
        tree_shader.indices[i*3 + 2] = face.verts.z;
    }

    ground_shader.num_vertices = (GROUND_NUM_VERTICES_SIDE+1) * (GROUND_NUM_VERTICES_SIDE+1);
    ground_shader.vertices = (EsVertex*) SDL_malloc(ground_shader.num_vertices * sizeof(EsVertex));
    ground_shader.num_indices = ground_shader.num_vertices * 6;
    ground_shader.indices = (Uint32*) SDL_calloc(ground_shader.num_indices, sizeof(Uint32));
    float tex_x = 0.0;
    float tex_y = 0.0;
    for (Uint32 i=0; i<GROUND_NUM_VERTICES_SIDE+1; i++) {
        float x = 3.0f * GRASS_RADIUS * ((float) i) / ((float) GROUND_NUM_VERTICES_SIDE);
        x -= GRASS_RADIUS*1.5f;
        for (Uint32 j=0; j<GROUND_NUM_VERTICES_SIDE+1; j++) {
            float z = 3.0f * GRASS_RADIUS * ((float) j) / ((float) GROUND_NUM_VERTICES_SIDE);
            z -= GRASS_RADIUS*1.5f;
            Uint32 index = i*(GROUND_NUM_VERTICES_SIDE+1) + j;
            float y = 1.0f * stb_perlin_noise3(x/10.0f, 0, z/10.0f, 0, 0, 0);
            y -= 0.3f;
            ground_shader.vertices[index].pos = build_vec3(x, y, z);
            ground_shader.vertices[index].color = build_vec3(0, 0, 0);
            ground_shader.vertices[index].tex = build_vec2(tex_x, tex_y);
            ground_shader.vertices[index].normal = build_vec3(0, 1, 0);
            if (tex_y == 1.0)
                tex_y = 0.0;
            else
                tex_y = 1.0;
        }
        if (tex_x == 1.0)
            tex_x = 0.0;
        else
            tex_x = 1.0;
    }
    for (Uint32 i=0; i<GROUND_NUM_VERTICES_SIDE; i++) {
        for (Uint32 j=0; j<GROUND_NUM_VERTICES_SIDE; j++) {
            Uint32 index = 6 * (i*(GROUND_NUM_VERTICES_SIDE+1) + j);
            Uint32 v1 = (i+0)*(GROUND_NUM_VERTICES_SIDE+1) + (j+0);
            Uint32 v2 = (i+0)*(GROUND_NUM_VERTICES_SIDE+1) + (j+1);
            Uint32 v3 = (i+1)*(GROUND_NUM_VERTICES_SIDE+1) + (j+1);
            Uint32 v4 = (i+1)*(GROUND_NUM_VERTICES_SIDE+1) + (j+0);
            ground_shader.indices[index+0] = v2;
            ground_shader.indices[index+1] = v4;
            ground_shader.indices[index+2] = v1;
            ground_shader.indices[index+3] = v2;
            ground_shader.indices[index+4] = v3;
            ground_shader.indices[index+5] = v4;
        }
    }

    painter->shaders[0] = tree_shader;
    painter->shaders[1] = grass_shader;
    painter->shaders[2] = ground_shader;

    painter->uniform_buffer_object.model = identity_mat4();
    painter->camera_fov = 45.0f;
    painter->uniform_buffer_object.proj = perspective_projection(deg_to_rad(painter->camera_fov), (1024.0f/768.0f), 0.1f, 200.0f);

    return SDL_TRUE;
}

SDL_bool painter_initialise(EsPainter* painter) {
    SDL_bool sdl_result;

    sdl_result = _painter_initialise_sdl_window(painter, "Easel");
    if (!sdl_result) return SDL_FALSE;
    painter->num_shaders = 3;
    painter->skybox_shader = (ShaderData*) SDL_malloc(1 * sizeof(ShaderData));
    painter->shaders = (ShaderData*) SDL_malloc(painter->num_shaders * sizeof(ShaderData));
    sdl_result = _painter_load_data(painter);
    if (!sdl_result) return SDL_FALSE;
    sdl_result = _painter_init_instance(painter);
    if (!sdl_result) return SDL_FALSE;
    sdl_result = _painter_select_physical_device(painter);
    if (!sdl_result) return SDL_FALSE;
    sdl_result = _painter_create_swapchain(painter);
    if (!sdl_result) return SDL_FALSE;
    sdl_result = _painter_create_synchronisation_elements(painter);
    if (!sdl_result) return SDL_FALSE;

    painter->frame_index = 0;
    painter->buffer_resized = SDL_FALSE;

    return SDL_TRUE;
}

SDL_bool _painter_create_swapchain(EsPainter* painter) {
    VkResult result;
    SDL_bool sdl_result;

    sdl_result = _painter_swapchain_renderpass_init(painter);
    if (!sdl_result) return SDL_FALSE;

    for (Uint32 i=0; i<painter->num_shaders; i++) {
        sdl_result = _painter_init_shader_data(painter, &painter->shaders[i], SDL_FALSE);
        if (!sdl_result) return SDL_FALSE;
    }
    sdl_result = _painter_init_shader_data(painter, painter->skybox_shader, SDL_TRUE);
    if (!sdl_result) return SDL_FALSE;

    painter->uniform_buffer_size = sizeof(UniformBufferObject);
    painter->uniform_buffers = (VkBuffer*) SDL_malloc(painter->swapchain_image_count * sizeof(VkBuffer));
    painter->uniform_buffers_memory = (VkDeviceMemory*) SDL_malloc(painter->swapchain_image_count * sizeof(VkDeviceMemory));
    for (Uint32 i=0; i<painter->swapchain_image_count; i++) {
        sdl_result = _painter_create_buffer(painter, painter->uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &painter->uniform_buffers[i], &painter->uniform_buffers_memory[i]);
        if (!sdl_result) return SDL_FALSE;
    }

    for (Uint32 i=0; i<painter->num_shaders; i++) {
        sdl_result = _painter_create_descriptor_sets(painter, &painter->shaders[i]);
        if (!sdl_result) _painter_custom_error("Setup Error", "Could not create descriptor sets");
    }
    sdl_result = _painter_create_descriptor_sets(painter, painter->skybox_shader);
    if (!sdl_result) _painter_custom_error("Setup Error", "Could not create descriptor sets");

    sdl_result = _painter_create_framebuffers(painter);
    if (!sdl_result) _painter_custom_error("Setup Error", "Could not create framebuffers");

    for (Uint32 i=0; i<painter->swapchain_image_count; i++) {
        VkCommandBufferBeginInfo command_buffer_begin_info;
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_info.pNext = NULL;
        command_buffer_begin_info.flags = 0;
        command_buffer_begin_info.pInheritanceInfo = NULL;
        result = vkBeginCommandBuffer(painter->command_buffers[i], &command_buffer_begin_info);
        if (result != VK_SUCCESS) {
            warehouse_error_popup("Error in Vulkan Setup.", "Could not begin command buffers");
            painter_cleanup(painter);
            return SDL_FALSE;
        }
        VkRenderPassBeginInfo render_pass_begin_info;
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.pNext = NULL;
        render_pass_begin_info.renderPass = painter->render_pass;
        render_pass_begin_info.framebuffer = painter->swapchain_framebuffers[i];
        render_pass_begin_info.renderArea.offset.x = 0;
        render_pass_begin_info.renderArea.offset.y = 0;
        render_pass_begin_info.renderArea.extent = painter->swapchain_extent;
        
        VkClearValue clear_values[2];
        VkClearColorValue color_value0 = { 1.0f, 0.0f, 0.0f, 1.0f };
        VkClearColorValue color_value1 = { 0.0f, 0.0f, 0.0f, 0.0f };
        VkClearDepthStencilValue depth_value0 = { 0.0f, 0 };
        VkClearDepthStencilValue depth_value1 = { 1.0f, 0 };
        clear_values[0].color = color_value0;
        clear_values[1].color = color_value1;
        clear_values[0].depthStencil = depth_value0;
        clear_values[1].depthStencil = depth_value1;
        render_pass_begin_info.clearValueCount = 2;
        render_pass_begin_info.pClearValues = clear_values;
        VkBuffer vertex_buffers[1];
        VkDeviceSize offsets[1];
        offsets[0] = 0;

        vkCmdBeginRenderPass(painter->command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        for (Uint32 j=0; j<painter->num_shaders; j++) {
            vertex_buffers[0] = painter->shaders[j].vertex_buffer;
            vkCmdBindPipeline(painter->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, painter->shaders[j].pipeline);
            vkCmdBindVertexBuffers(painter->command_buffers[i], 0, 1, vertex_buffers, offsets);
            vkCmdBindIndexBuffer(painter->command_buffers[i], painter->shaders[j].index_buffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(painter->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, painter->shaders[j].pipeline_layout, 0, 1, &painter->shaders[j].descriptor_sets[i], 0, NULL);
            vkCmdDrawIndexed(painter->command_buffers[i], painter->shaders[j].num_indices, 1, 0, 0, 0);
        }
        vertex_buffers[0] = painter->skybox_shader->vertex_buffer;
        vkCmdBindPipeline(painter->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, painter->skybox_shader->pipeline);
        vkCmdBindVertexBuffers(painter->command_buffers[i], 0, 1, vertex_buffers, offsets);
        vkCmdBindIndexBuffer(painter->command_buffers[i], painter->skybox_shader->index_buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(painter->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, painter->skybox_shader->pipeline_layout, 0, 1, &painter->skybox_shader->descriptor_sets[i], 0, NULL);
        vkCmdDrawIndexed(painter->command_buffers[i], painter->skybox_shader->num_indices, 1, 0, 0, 0);

        vkCmdEndRenderPass(painter->command_buffers[i]);
        result = vkEndCommandBuffer(painter->command_buffers[i]);
        if (result != VK_SUCCESS) {
            warehouse_error_popup("Error in Vulkan Setup.", "Could not end command buffers");
            painter_cleanup(painter);
            return SDL_FALSE;
        }
    }
    return SDL_TRUE;
}

SDL_bool painter_paint_frame(EsPainter* painter) {
    // TODO (20 Oct 2020 sam): We need to handle the case of minimized frames.
    Uint32 image_index;
    VkResult result;
    SDL_bool sdl_result;

    vkWaitForFences(painter->device, 1, &painter->in_flight_fences[painter->frame_index], VK_TRUE, UINT64_MAX);
    result = vkAcquireNextImageKHR(painter->device, painter->swapchain, UINT64_MAX, painter->image_available_semaphores[painter->frame_index], VK_NULL_HANDLE, &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || painter->buffer_resized) {
        painter->buffer_resized = SDL_FALSE;
        sdl_result = _painter_recreate_swapchain(painter);
        if (!sdl_result) {
            return SDL_FALSE;
        }
    } else if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Rendering.", "Could not acquire next image");
        painter_cleanup(painter);
        return SDL_FALSE;
    }

    painter->uniform_buffer_object.time = (float) (SDL_GetTicks()/1000.0f);
    vec3 target = painter->world->target;
    painter->camera_position = painter->world->position;
    painter->uniform_buffer_object.camera_position = painter->camera_position;
    painter->uniform_buffer_object.view = look_at(painter->camera_position, target, painter->world->up_axis);

    void* uniform_data;
    result = vkMapMemory(painter->device, painter->uniform_buffers_memory[image_index], 0, painter->uniform_buffer_size, 0, &uniform_data);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Rendering.", "Could not map uniform memory");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    SDL_memcpy(uniform_data, &painter->uniform_buffer_object, (size_t) painter->uniform_buffer_size);
    vkUnmapMemory(painter->device, painter->uniform_buffers_memory[image_index]);

    if (painter->images_in_flight[image_index] != VK_NULL_HANDLE)
        vkWaitForFences(painter->device, 1, &painter->images_in_flight[image_index], VK_TRUE, UINT64_MAX);
    painter->images_in_flight[image_index] = painter->in_flight_fences[painter->frame_index];
    VkSemaphore wait_semaphores[1];
    wait_semaphores[0] = painter->image_available_semaphores[painter->frame_index];
    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signal_semaphores[1];
    signal_semaphores[0] = painter->render_finished_semaphores[painter->frame_index];
    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = NULL;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &painter->command_buffers[image_index];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;
    vkResetFences(painter->device, 1, &painter->in_flight_fences[painter->frame_index]);
    result = vkQueueSubmit(painter->graphics_queue, 1, &submit_info, painter->in_flight_fences[painter->frame_index]);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Rendering.", "Could not submit to graphics queue");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    VkSwapchainKHR swapchains[1];
    swapchains[0] = painter->swapchain;
    VkPresentInfoKHR present_info;
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = NULL;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = NULL;
    result = vkQueuePresentKHR(painter->presentation_queue, &present_info);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Rendering.", "Could not present queue");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    painter->frame_index = (painter->frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
    return SDL_TRUE;
}
