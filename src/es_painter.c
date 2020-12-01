#include "es_painter.h"
#include "SDL_vulkan.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"
#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader_c.h"

#include <stdlib.h>

#define GRASS_INSTANCES 1000
#define GRASS_NUM_VERTICES 4
#define GRASS_MODEL_PATH "data/obj/grass3.obj"
#define GRASS_MODEL_TEXTURE_PATH "data/img/grass3.png"
#define SKYBOX_MODEL_PATH "data/obj/skybox.obj"
#define SKYBOX_MODEL_TEXTURE_PATH4 "data/img/skybox/front.jpg"
#define SKYBOX_MODEL_TEXTURE_PATH5 "data/img/skybox/back.jpg"
#define SKYBOX_MODEL_TEXTURE_PATH2 "data/img/skybox/top.jpg"
#define SKYBOX_MODEL_TEXTURE_PATH3 "data/img/skybox/bottom.jpg"
#define SKYBOX_MODEL_TEXTURE_PATH1 "data/img/skybox/left.jpg"
#define SKYBOX_MODEL_TEXTURE_PATH0 "data/img/skybox/right.jpg"
#define TREE_MODEL_PATH "data/obj/tree.obj"
#define TREE_MODEL_TEXTURE_PATH "data/img/tree.png"

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
    ret = tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials,
                            &num_materials, GRASS_MODEL_PATH, _painter_read_obj_file, flags);
    if (ret != TINYOBJ_SUCCESS) {
        warehouse_error_popup("Error in Setup.", "Could not load model");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    painter->grass_shader->num_vertices = attrib.num_vertices * GRASS_INSTANCES;
    painter->grass_shader->vertices = (EsVertex*) SDL_malloc(painter->grass_shader->num_vertices * sizeof(EsVertex));
    painter->grass_shader->num_indices = attrib.num_faces * GRASS_INSTANCES;
    painter->grass_shader->indices = (Uint32*) SDL_malloc(painter->grass_shader->num_indices * sizeof(Uint32));
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
        painter->grass_shader->vertices[i] = vert;
    }
    for (Uint32 i=0; i<attrib.num_faces; i++) {
        tinyobj_vertex_index_t face = attrib.faces[i];
        painter->grass_shader->indices[i] = face.v_idx;
        painter->grass_shader->vertices[face.v_idx].tex.x = attrib.texcoords[face.vt_idx*2 + 0];
        painter->grass_shader->vertices[face.v_idx].tex.y = attrib.texcoords[face.vt_idx*2 + 1];
    }
    for (Uint32 i=1; i<GRASS_INSTANCES; i++) {
        float x = ((float) rand() / (float) RAND_MAX * 3.0f) - 1.5f;
        float y = ((float) rand() / (float) RAND_MAX * 0.1f) - 0.05f;
        float z = ((float) rand() / (float) RAND_MAX * 3.0f) - 1.5f;
        for (Uint32 j=0; j<GRASS_NUM_VERTICES; j++) {
            EsVertex vert = painter->grass_shader->vertices[j];
            vert.pos.x += x;
            vert.pos.y += y;
            vert.pos.z += z;        
            painter->grass_shader->vertices[i*GRASS_NUM_VERTICES + j] = vert;
        }
        for (Uint32 j=0; j<attrib.num_faces; j++) {
            painter->grass_shader->indices[i*attrib.num_faces + j] = i*GRASS_NUM_VERTICES + painter->grass_shader->indices[j];
        }
    }
    // SDL_Log("perlin: %f", stb_perlin_noise3(0.01, 0.2, 2.1, 0, 0, 0));
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

    ret = tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials,
                            &num_materials, TREE_MODEL_PATH, _painter_read_obj_file, flags);
    if (ret != TINYOBJ_SUCCESS) {
        warehouse_error_popup("Error in Setup.", "Could not load model");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    painter->tree_shader->num_vertices = attrib.num_vertices;
    painter->tree_shader->vertices = (EsVertex*) SDL_malloc(painter->tree_shader->num_vertices * sizeof(EsVertex));
    painter->tree_shader->num_indices = attrib.num_faces;
    painter->tree_shader->indices = (Uint32*) SDL_malloc(painter->tree_shader->num_indices * sizeof(Uint32));
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
        vert.tex.x = 0.0f;
        vert.tex.y = 0.0f;
        painter->tree_shader->vertices[i] = vert;
    }
    for (Uint32 i=0; i<attrib.num_faces; i++) {
        tinyobj_vertex_index_t face = attrib.faces[i];
        painter->tree_shader->indices[i] = face.v_idx;
        // painter->tree_shader->vertices[face.v_idx].tex.x = attrib.texcoords[face.vt_idx*2 + 0];
        // painter->tree_shader->vertices[face.v_idx].tex.y = attrib.texcoords[face.vt_idx*2 + 1];
    }
    // SDL_Log("perlin: %f", stb_perlin_noise3(0.01, 0.2, 2.1, 0, 0, 0));
    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, num_shapes);
    tinyobj_materials_free(materials, num_materials);

    painter->uniform_buffer_object.model = identity_mat4();
    painter->camera_fov = 45.0f;
    painter->uniform_buffer_object.proj = perspective_projection(deg_to_rad(painter->camera_fov), (1024.0f/768.0f), 0.1f, 20.0f);

    return SDL_TRUE;
}

SDL_bool painter_initialise(EsPainter* painter) {
    SDL_bool sdl_result;

    sdl_result = _painter_initialise_sdl_window(painter, "Easel");
    if (!sdl_result) return SDL_FALSE;
    painter->grass_shader = (ShaderData*) SDL_malloc(1 * sizeof(ShaderData));
    painter->skybox_shader = (ShaderData*) SDL_malloc(1 * sizeof(ShaderData));
    painter->tree_shader = (ShaderData*) SDL_malloc(1 * sizeof(ShaderData));
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

    sdl_result = _painter_load_shaders(painter, painter->grass_shader, "data/spirv/grass_vertex.spv", "data/spirv/grass_fragment.spv");
    if (!sdl_result) return SDL_FALSE;
    sdl_result = _painter_create_descriptor_set_layout(painter, painter->grass_shader);
    if (!sdl_result) return SDL_FALSE;
    sdl_result = _painter_create_pipeline(painter, painter->grass_shader);
    if (!sdl_result) return SDL_FALSE;

    sdl_result = _painter_load_shaders(painter, painter->skybox_shader, "data/spirv/skybox_vertex.spv", "data/spirv/skybox_fragment.spv");
    if (!sdl_result) return SDL_FALSE;
    sdl_result = _painter_create_descriptor_set_layout(painter, painter->skybox_shader);
    if (!sdl_result) return SDL_FALSE;
    sdl_result = _painter_create_pipeline(painter, painter->skybox_shader);
    if (!sdl_result) return SDL_FALSE;

    sdl_result = _painter_load_shaders(painter, painter->tree_shader, "data/spirv/tree_vertex.spv", "data/spirv/tree_fragment.spv");
    if (!sdl_result) return SDL_FALSE;
    sdl_result = _painter_create_descriptor_set_layout(painter, painter->tree_shader);
    if (!sdl_result) return SDL_FALSE;
    sdl_result = _painter_create_pipeline(painter, painter->tree_shader);
    if (!sdl_result) return SDL_FALSE;

    sdl_result = _painter_load_image_and_sampler(painter, GRASS_MODEL_TEXTURE_PATH, painter->grass_shader);
    if (!sdl_result) return _painter_custom_error("Setup Error", "Grass texture loading");
    sdl_result = _painter_load_cubemap_and_sampler(painter, SKYBOX_MODEL_TEXTURE_PATH0, SKYBOX_MODEL_TEXTURE_PATH1, SKYBOX_MODEL_TEXTURE_PATH2, SKYBOX_MODEL_TEXTURE_PATH3, SKYBOX_MODEL_TEXTURE_PATH4, SKYBOX_MODEL_TEXTURE_PATH5, painter->skybox_shader);
    if (!sdl_result) return _painter_custom_error("Setup Error", "skybox texture loading");
    sdl_result = _painter_load_image_and_sampler(painter, TREE_MODEL_TEXTURE_PATH, painter->tree_shader);
    if (!sdl_result) return _painter_custom_error("Setup Error", "Grass texture loading");

    SDL_Log("creating grass buffers\n");
    painter->grass_shader->vertex_staging_buffer_size = painter->grass_shader->num_vertices * sizeof(EsVertex);
    VkMemoryPropertyFlags vertex_staging_property_flags =  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    // TODO (21 Oct 2020 sam): Use a single vkAllocateMemory for both buffers, and manage memory using
    // the offsets and things.
    sdl_result = _painter_create_buffer(painter, painter->grass_shader->vertex_staging_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vertex_staging_property_flags, &painter->grass_shader->vertex_staging_buffer, &painter->grass_shader->vertex_staging_buffer_memory);
    if (!sdl_result) return SDL_FALSE;
    painter->grass_shader->vertex_buffer_size = painter->grass_shader->num_vertices * sizeof(EsVertex);
    VkMemoryPropertyFlags vertex_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    sdl_result = _painter_create_buffer(painter, painter->grass_shader->vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertex_property_flags, &painter->grass_shader->vertex_buffer, &painter->grass_shader->vertex_buffer_memory);
    if (!sdl_result) return SDL_FALSE;

    painter->grass_shader->index_staging_buffer_size = painter->grass_shader->num_indices * sizeof(Uint32);
    VkMemoryPropertyFlags index_staging_property_flags =  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    // TODO (21 Oct 2020 sam): Use a single vkAllocateMemory for both buffers, and manage memory using
    // the offsets and things.
    SDL_Log("num vertices = %i num indices = %i\n", painter->grass_shader->num_vertices, painter->grass_shader->num_indices);
    sdl_result = _painter_create_buffer(painter, painter->grass_shader->index_staging_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, index_staging_property_flags, &painter->grass_shader->index_staging_buffer, &painter->grass_shader->index_staging_buffer_memory);
    if (!sdl_result) return SDL_FALSE;
    painter->grass_shader->index_buffer_size = painter->grass_shader->num_indices * sizeof(Uint32);
    VkMemoryPropertyFlags index_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    sdl_result = _painter_create_buffer(painter, painter->grass_shader->index_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, index_property_flags, &painter->grass_shader->index_buffer, &painter->grass_shader->index_buffer_memory);
    if (!sdl_result) return SDL_FALSE;

    sdl_result = _painter_load_buffer_via_staging(painter, painter->grass_shader->vertices, &painter->grass_shader->vertex_staging_buffer_memory, &painter->grass_shader->vertex_staging_buffer, &painter->grass_shader->vertex_buffer, painter->grass_shader->vertex_staging_buffer_size);
    if (!sdl_result) _painter_cleanup_error(painter, "Setup Error", "Could not copy vertices to buffer.");
    sdl_result = _painter_load_buffer_via_staging(painter, painter->grass_shader->indices, &painter->grass_shader->index_staging_buffer_memory, &painter->grass_shader->index_staging_buffer, &painter->grass_shader->index_buffer, painter->grass_shader->index_staging_buffer_size);
    if (!sdl_result) _painter_cleanup_error(painter, "Setup Error", "Could not copy indices to buffer.");

    SDL_Log("creating skybox buffers\n");
    painter->skybox_shader->vertex_staging_buffer_size = painter->skybox_shader->num_vertices * sizeof(EsVertex);
    vertex_staging_property_flags =  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    // TODO (21 Oct 2020 sam): Use a single vkAllocateMemory for both buffers, and manage memory using
    // the offsets and things.
    sdl_result = _painter_create_buffer(painter, painter->skybox_shader->vertex_staging_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vertex_staging_property_flags, &painter->skybox_shader->vertex_staging_buffer, &painter->skybox_shader->vertex_staging_buffer_memory);
    if (!sdl_result) return SDL_FALSE;
    painter->skybox_shader->vertex_buffer_size = painter->skybox_shader->num_vertices * sizeof(EsVertex);
    vertex_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    sdl_result = _painter_create_buffer(painter, painter->skybox_shader->vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertex_property_flags, &painter->skybox_shader->vertex_buffer, &painter->skybox_shader->vertex_buffer_memory);
    if (!sdl_result) return SDL_FALSE;

    painter->skybox_shader->index_staging_buffer_size = painter->skybox_shader->num_indices * sizeof(Uint32);
    index_staging_property_flags =  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    // TODO (21 Oct 2020 sam): Use a single vkAllocateMemory for both buffers, and manage memory using
    // the offsets and things.
    SDL_Log("num vertices = %i num indices = %i\n", painter->skybox_shader->num_vertices, painter->skybox_shader->num_indices);
    sdl_result = _painter_create_buffer(painter, painter->skybox_shader->index_staging_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, index_staging_property_flags, &painter->skybox_shader->index_staging_buffer, &painter->skybox_shader->index_staging_buffer_memory);
    if (!sdl_result) return SDL_FALSE;
    painter->skybox_shader->index_buffer_size = painter->skybox_shader->num_indices * sizeof(Uint32);
    index_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    sdl_result = _painter_create_buffer(painter, painter->skybox_shader->index_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, index_property_flags, &painter->skybox_shader->index_buffer, &painter->skybox_shader->index_buffer_memory);
    if (!sdl_result) return SDL_FALSE;

    sdl_result = _painter_load_buffer_via_staging(painter, painter->skybox_shader->vertices, &painter->skybox_shader->vertex_staging_buffer_memory, &painter->skybox_shader->vertex_staging_buffer, &painter->skybox_shader->vertex_buffer, painter->skybox_shader->vertex_staging_buffer_size);
    if (!sdl_result) _painter_cleanup_error(painter, "Setup Error", "Could not copy vertices to buffer.");
    sdl_result = _painter_load_buffer_via_staging(painter, painter->skybox_shader->indices, &painter->skybox_shader->index_staging_buffer_memory, &painter->skybox_shader->index_staging_buffer, &painter->skybox_shader->index_buffer, painter->skybox_shader->index_staging_buffer_size);
    if (!sdl_result) _painter_cleanup_error(painter, "Setup Error", "Could not copy indices to buffer.");

    SDL_Log("creating tree buffers\n");
    painter->tree_shader->vertex_staging_buffer_size = painter->tree_shader->num_vertices * sizeof(EsVertex);
    vertex_staging_property_flags =  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    // TODO (21 Oct 2020 sam): Use a single vkAllocateMemory for both buffers, and manage memory using
    // the offsets and things.
    sdl_result = _painter_create_buffer(painter, painter->tree_shader->vertex_staging_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vertex_staging_property_flags, &painter->tree_shader->vertex_staging_buffer, &painter->tree_shader->vertex_staging_buffer_memory);
    if (!sdl_result) return SDL_FALSE;
    painter->tree_shader->vertex_buffer_size = painter->tree_shader->num_vertices * sizeof(EsVertex);
    vertex_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    sdl_result = _painter_create_buffer(painter, painter->tree_shader->vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertex_property_flags, &painter->tree_shader->vertex_buffer, &painter->tree_shader->vertex_buffer_memory);
    if (!sdl_result) return SDL_FALSE;

    painter->tree_shader->index_staging_buffer_size = painter->tree_shader->num_indices * sizeof(Uint32);
    index_staging_property_flags =  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    // TODO (21 Oct 2020 sam): Use a single vkAllocateMemory for both buffers, and manage memory using
    // the offsets and things.
    SDL_Log("num vertices = %i num indices = %i\n", painter->tree_shader->num_vertices, painter->tree_shader->num_indices);
    sdl_result = _painter_create_buffer(painter, painter->tree_shader->index_staging_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, index_staging_property_flags, &painter->tree_shader->index_staging_buffer, &painter->tree_shader->index_staging_buffer_memory);
    if (!sdl_result) return SDL_FALSE;
    painter->tree_shader->index_buffer_size = painter->tree_shader->num_indices * sizeof(Uint32);
    index_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    sdl_result = _painter_create_buffer(painter, painter->tree_shader->index_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, index_property_flags, &painter->tree_shader->index_buffer, &painter->tree_shader->index_buffer_memory);
    if (!sdl_result) return SDL_FALSE;

    sdl_result = _painter_load_buffer_via_staging(painter, painter->tree_shader->vertices, &painter->tree_shader->vertex_staging_buffer_memory, &painter->tree_shader->vertex_staging_buffer, &painter->tree_shader->vertex_buffer, painter->tree_shader->vertex_staging_buffer_size);
    if (!sdl_result) _painter_cleanup_error(painter, "Setup Error", "Could not copy vertices to buffer.");
    sdl_result = _painter_load_buffer_via_staging(painter, painter->tree_shader->indices, &painter->tree_shader->index_staging_buffer_memory, &painter->tree_shader->index_staging_buffer, &painter->tree_shader->index_buffer, painter->tree_shader->index_staging_buffer_size);
    if (!sdl_result) _painter_cleanup_error(painter, "Setup Error", "Could not copy indices to buffer.");

    painter->uniform_buffer_size = sizeof(UniformBufferObject);
    painter->uniform_buffers = (VkBuffer*) SDL_malloc(painter->swapchain_image_count * sizeof(VkBuffer));
    painter->uniform_buffers_memory = (VkDeviceMemory*) SDL_malloc(painter->swapchain_image_count * sizeof(VkDeviceMemory));
    for (Uint32 i=0; i<painter->swapchain_image_count; i++) {
        sdl_result = _painter_create_buffer(painter, painter->uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &painter->uniform_buffers[i], &painter->uniform_buffers_memory[i]);
        if (!sdl_result) return SDL_FALSE;
    }

    sdl_result = _painter_create_descriptor_sets(painter, painter->grass_shader);
    if (!sdl_result) _painter_custom_error("Setup Error", "Could not create descriptor sets");
    sdl_result = _painter_create_descriptor_sets(painter, painter->skybox_shader);
    if (!sdl_result) _painter_custom_error("Setup Error", "Could not create descriptor sets");
    sdl_result = _painter_create_descriptor_sets(painter, painter->tree_shader);
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

        vertex_buffers[0] = painter->tree_shader->vertex_buffer;
        vkCmdBindPipeline(painter->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, painter->tree_shader->pipeline);
        vkCmdBindVertexBuffers(painter->command_buffers[i], 0, 1, vertex_buffers, offsets);
        vkCmdBindIndexBuffer(painter->command_buffers[i], painter->tree_shader->index_buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(painter->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, painter->tree_shader->pipeline_layout, 0, 1, &painter->tree_shader->descriptor_sets[i], 0, NULL);
        vkCmdDrawIndexed(painter->command_buffers[i], painter->tree_shader->num_indices, 1, 0, 0, 0);

        vertex_buffers[0] = painter->grass_shader->vertex_buffer;
        vkCmdBindPipeline(painter->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, painter->grass_shader->pipeline);
        vkCmdBindVertexBuffers(painter->command_buffers[i], 0, 1, vertex_buffers, offsets);
        vkCmdBindIndexBuffer(painter->command_buffers[i], painter->grass_shader->index_buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(painter->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, painter->grass_shader->pipeline_layout, 0, 1, &painter->grass_shader->descriptor_sets[i], 0, NULL);
        vkCmdDrawIndexed(painter->command_buffers[i], painter->grass_shader->num_indices, 1, 0, 0, 0);

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
    vec3 target = build_vec3(0.0f, 5.0f, 0.0f);
    vec3 base_camera = build_vec3(0.0f, 1.0f, 5.0f);
    float angle = ((float) M_PI) * SDL_sinf(painter->uniform_buffer_object.time / 5.2f);
    angle = 0.0f;
    painter->camera_position = rotate_about_origin_yaxis(base_camera, angle);
    painter->uniform_buffer_object.camera_position = painter->camera_position;
    painter->uniform_buffer_object.view = look_at_y(painter->camera_position, target);

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
