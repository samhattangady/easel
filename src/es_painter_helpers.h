extern void _painter_cleanup_swapchain(EsPainter* painter);
extern void _painter_shader_cleanup(EsPainter* painter, ShaderData* shader);
extern SDL_bool _painter_recreate_swapchain(EsPainter* painter);
extern SDL_bool _painter_create_buffer(EsPainter* painter, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
extern Uint32 _painter_find_memory_type(EsPainter* painter, VkMemoryPropertyFlags property_flags, VkMemoryRequirements* memory_requirements);
extern SDL_bool _painter_transition_image_layout(EsPainter* painter, VkImage* image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, Uint32 mip_levels, Uint32 layer_count);
extern SDL_bool _painter_copy_buffer_to_image(EsPainter* painter, VkBuffer* buffer, VkImage* image, Uint32 width, Uint32 height);
extern SDL_bool _painter_copy_cubemap_buffer_to_image(EsPainter* painter, VkBuffer* buffer, VkImage* image, Uint32 width, Uint32 height);
extern SDL_bool _painter_create_image(EsPainter* painter, Uint32 width, Uint32 height, Uint32 mip_levels, VkSampleCountFlagBits num_samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* image_memory, Uint32 array_layers, SDL_bool is_cube);
extern SDL_bool _painter_create_image_view(EsPainter* painter, VkFormat format, VkImageAspectFlags aspect_flags, VkImageViewType image_view_type, Uint32 mip_levels, VkImage* image, VkImageView* image_view);
extern VkCommandBuffer _painter_begin_single_use_command_buffer(EsPainter* painter);
extern SDL_bool _painter_end_single_use_command_buffer(EsPainter* painter, VkCommandBuffer* command_buffer);
extern SDL_bool _painter_generate_mipmaps(EsPainter* painter, VkImage* image, VkFormat image_format, Uint32 width, Uint32 height, Uint32 mip_levels, SDL_bool is_cubemap);
extern SDL_bool _painter_initialise_sdl_window(EsPainter* painter, const char* window_name);
extern SDL_bool _painter_init_instance(EsPainter* painter);
extern SDL_bool _painter_select_physical_device(EsPainter* painter);
extern SDL_bool _painter_create_synchronisation_elements(EsPainter* painter);
extern SDL_bool _painter_create_device_and_queues(EsPainter* painter);
extern SDL_bool _painter_load_shaders(EsPainter* painter, ShaderData* shader);
extern SDL_bool _painter_create_descriptor_set_layout(EsPainter* painter, ShaderData* shader);
extern SDL_bool _painter_create_pipeline(EsPainter* painter, ShaderData* shader);
extern SDL_bool _painter_load_buffer_via_staging(EsPainter* painter, void* data, VkDeviceMemory* memory, VkBuffer* src, VkBuffer* dst, Uint32 size);
extern void _painter_read_obj_file(const char* filename, const int is_mtl, const char *obj_filename, char** data, size_t* len);
extern SDL_bool _painter_swapchain_renderpass_init(EsPainter* painter);
extern SDL_bool _painter_custom_error(const char* header, const char* message);
extern SDL_bool _painter_cleanup_error(EsPainter* painter, const char* header, const char* message);
extern SDL_bool _painter_load_image_and_sampler(EsPainter* painter, const char* filepath, ShaderData* shader);
extern SDL_bool _painter_load_cubemap_and_sampler(EsPainter* painter, const char* filepath0, const char* filepath1, const char* filepath2, const char* filepath3, const char* filepath4, const char* filepath5, ShaderData* shader);
extern SDL_bool _painter_create_framebuffers(EsPainter* painter);
extern SDL_bool _painter_create_descriptor_sets(EsPainter* painter, ShaderData* shader);
extern SDL_bool _painter_init_shader_data(EsPainter* painter, ShaderData* shader, SDL_bool is_skybox);
extern SDL_bool _painter_load_buffer_from_geom(EsPainter* painter, EsGeometry* geom, ShaderData* shader);
SDL_bool _painter_create_swapchain(EsPainter* painter);

SDL_bool _painter_load_buffer_from_geom(EsPainter* painter, EsGeometry* geom, ShaderData* shader) {
    SDL_bool sdl_result;
    // TODO (09 Dec 2020 sam): We should be reallocating memory and buffers if there is not enought
    // memory available.
    if (geom->num_vertices > 100000 || geom->num_faces*3 > 1000000) {
        SDL_Log("Not enough memory. Sorry");
        return SDL_FALSE;
    }
    shader->num_vertices = geom->num_vertices;
    shader->vertices = (EsVertex*) SDL_malloc(shader->num_vertices * sizeof(EsVertex));
    shader->num_indices = geom->num_faces * 3;
    shader->indices = (Uint32*) SDL_malloc(shader->num_indices * sizeof(Uint32));
    shader->vertex_staging_buffer_size = shader->num_vertices * sizeof(EsVertex);
    shader->index_staging_buffer_size = shader->num_indices * sizeof(Uint32);
    for (Uint32 i=0; i<geom->num_vertices; i++) {
        EsVertex vert;
        vert.pos = geom->vertices[i];
        vert.color.x = 0.0;
        vert.color.y = 0.0;
        vert.color.z = 0.0;
        shader->vertices[i] = vert;
    }
    for (Uint32 i=0; i<geom->num_faces; i++) {
        EsFace face = geom->faces[i];
        EsVertex vert_x = shader->vertices[face.verts.x];
        EsVertex vert_y = shader->vertices[face.verts.y];
        EsVertex vert_z = shader->vertices[face.verts.z];
        vert_x.tex = geom->textures[face.texs.x];
        vert_y.tex = geom->textures[face.texs.y];
        vert_z.tex = geom->textures[face.texs.z];
        vert_x.normal = geom->normals[face.norms.x];
        vert_y.normal = geom->normals[face.norms.y];
        vert_z.normal = geom->normals[face.norms.z];
        vert_x.color = geom->colors[face.cols.x];
        vert_y.color = geom->colors[face.cols.y];
        vert_z.color = geom->colors[face.cols.z];
        shader->vertices[face.verts.x] = vert_x;
        shader->vertices[face.verts.y] = vert_y;
        shader->vertices[face.verts.z] = vert_z;
        shader->indices[i*3 + 0] = face.verts.x;
        shader->indices[i*3 + 1] = face.verts.y;
        shader->indices[i*3 + 2] = face.verts.z;
    }

    sdl_result = _painter_load_buffer_via_staging(painter, shader->vertices, &shader->vertex_staging_buffer_memory, &shader->vertex_staging_buffer, &shader->vertex_buffer, shader->vertex_staging_buffer_size);
    if (!sdl_result) _painter_cleanup_error(painter, "Could not copy vertices to buffer.", shader->shader_name);
    sdl_result = _painter_load_buffer_via_staging(painter, shader->indices, &shader->index_staging_buffer_memory, &shader->index_staging_buffer, &shader->index_buffer, shader->index_staging_buffer_size);
    if (!sdl_result) _painter_cleanup_error(painter, "Could not copy indices to buffer.", shader->shader_name);
    SDL_free(shader->vertices);
    SDL_free(shader->indices);

    return SDL_TRUE;
}

SDL_bool _painter_create_framebuffers(EsPainter* painter) {
    VkResult result;
    painter->command_buffers = (VkCommandBuffer*) SDL_malloc(painter->swapchain_image_count * sizeof(VkCommandBuffer));
    VkCommandBufferAllocateInfo command_buffer_allocate_info;
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.pNext = NULL;
    command_buffer_allocate_info.commandPool = painter->command_pool;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = painter->swapchain_image_count;
    result = vkAllocateCommandBuffers(painter->device, &command_buffer_allocate_info, painter->command_buffers);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not allocate command buffers");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

SDL_bool _painter_create_descriptor_sets(EsPainter* painter, ShaderData* shader) {
    VkResult result;
    VkDescriptorPoolSize* descriptor_pool_size = (VkDescriptorPoolSize*) SDL_malloc(2*sizeof(VkDescriptorPoolSize));
    descriptor_pool_size[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_pool_size[0].descriptorCount = painter->swapchain_image_count;
    descriptor_pool_size[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_pool_size[1].descriptorCount = painter->swapchain_image_count;
    VkDescriptorPoolCreateInfo descriptor_pool_info;
    descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_info.pNext = NULL;
    descriptor_pool_info.flags = 0;
    descriptor_pool_info.maxSets = painter->swapchain_image_count;
    descriptor_pool_info.poolSizeCount = 2;
    descriptor_pool_info.pPoolSizes = descriptor_pool_size;
    result = vkCreateDescriptorPool(painter->device, &descriptor_pool_info, NULL, &shader->descriptor_pool);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create descriptor pool");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    VkDescriptorSetLayout* layouts = (VkDescriptorSetLayout*) SDL_malloc(painter->swapchain_image_count * sizeof(VkDescriptorSetLayout));
    for (Uint32 i=0; i<painter->swapchain_image_count; i++)
        layouts[i] = shader->descriptor_set_layout;
    shader->descriptor_sets = (VkDescriptorSet*) SDL_malloc(painter->swapchain_image_count * sizeof(VkDescriptorSet));
    VkDescriptorSetAllocateInfo descriptor_set_alloc_info;
    descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_set_alloc_info.pNext = NULL;
    descriptor_set_alloc_info.descriptorPool = shader->descriptor_pool;
    descriptor_set_alloc_info.descriptorSetCount = painter->swapchain_image_count;
    descriptor_set_alloc_info.pSetLayouts = layouts;
    result = vkAllocateDescriptorSets(painter->device, &descriptor_set_alloc_info, shader->descriptor_sets);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not allocate descriptor sets");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    for (Uint32 i=0; i<painter->swapchain_image_count; i++) {
        VkDescriptorBufferInfo buffer_info;
        buffer_info.buffer = painter->uniform_buffers[i];
        buffer_info.offset = 0;
        buffer_info.range = painter->uniform_buffer_size;
        VkDescriptorImageInfo image_info;
        image_info.sampler = shader->texture_sampler;
        image_info.imageView = shader->texture_image_view;
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkWriteDescriptorSet* descriptor_write = (VkWriteDescriptorSet*) SDL_malloc(2*sizeof(VkWriteDescriptorSet));
        descriptor_write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write[0].pNext = NULL;
        descriptor_write[0].dstSet = shader->descriptor_sets[i];
        descriptor_write[0].dstBinding = 0;
        descriptor_write[0].dstArrayElement = 0;
        descriptor_write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write[0].descriptorCount = 1;
        descriptor_write[0].pBufferInfo = &buffer_info;
        descriptor_write[0].pImageInfo = NULL;
        descriptor_write[0].pTexelBufferView = NULL;
        descriptor_write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write[1].pNext = NULL;
        descriptor_write[1].dstSet = shader->descriptor_sets[i];
        descriptor_write[1].dstBinding = 1;
        descriptor_write[1].dstArrayElement = 0;
        descriptor_write[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_write[1].descriptorCount = 1;
        descriptor_write[1].pBufferInfo = NULL;
        descriptor_write[1].pImageInfo = &image_info;
        descriptor_write[1].pTexelBufferView = NULL;
        vkUpdateDescriptorSets(painter->device, 2, descriptor_write, 0, NULL);
    }
    return SDL_TRUE;
}

SDL_bool _painter_load_cubemap_and_sampler(EsPainter* painter, const char* filepath0, const char* filepath1, const char* filepath2, const char* filepath3, const char* filepath4, const char* filepath5, ShaderData* shader) {
    VkResult result;
    SDL_bool sdl_result;
    int tex_width;
    int tex_height;
    int tex_channels;
    unsigned char* tex_data;
    VkBuffer tex_buffer;
    VkDeviceMemory tex_buffer_memory;
    VkDeviceSize tex_size;
    unsigned char* skybox_pixels[6];
    skybox_pixels[0] = stbi_load(filepath0, &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    skybox_pixels[1] = stbi_load(filepath1, &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    skybox_pixels[2] = stbi_load(filepath2, &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    skybox_pixels[3] = stbi_load(filepath3, &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    skybox_pixels[4] = stbi_load(filepath4, &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    skybox_pixels[5] = stbi_load(filepath5, &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    if (!skybox_pixels[0] || !skybox_pixels[1] || !skybox_pixels[2] || !skybox_pixels[3] || !skybox_pixels[4] || !skybox_pixels[5]) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not load texture");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    shader->mip_levels = (Uint32) (SDL_floor(warehouse_log_2(SDL_max((float) tex_width, (float) tex_height))) + 1.0f);
    tex_size = tex_width * tex_height * 4;
    sdl_result = _painter_create_buffer(painter, tex_size*6, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &tex_buffer, &tex_buffer_memory);
    if (!sdl_result) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create texture buffer");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    vkMapMemory(painter->device, tex_buffer_memory, 0, tex_size*6, 0, &tex_data);
    for (Uint32 i=0; i<6; i++)
        SDL_memcpy(tex_data + (tex_size*i), skybox_pixels[i], tex_size);
    vkUnmapMemory(painter->device, tex_buffer_memory);
    stbi_image_free(skybox_pixels[0]);
    stbi_image_free(skybox_pixels[1]);
    stbi_image_free(skybox_pixels[2]);
    stbi_image_free(skybox_pixels[3]);
    stbi_image_free(skybox_pixels[4]);
    stbi_image_free(skybox_pixels[5]);

    sdl_result = _painter_create_image(painter, tex_width, tex_height, shader->mip_levels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &shader->texture_image, &shader->texture_image_memory, 6, SDL_TRUE);
    if (!sdl_result) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create texture image");
        return SDL_FALSE;
    }
    sdl_result = _painter_transition_image_layout(painter, &shader->texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, shader->mip_levels, 6);
    if (!sdl_result) return SDL_FALSE;
    sdl_result = _painter_copy_cubemap_buffer_to_image(painter, &tex_buffer, &shader->texture_image, tex_width, tex_height);
    if (!sdl_result) return SDL_FALSE;
    vkDestroyBuffer(painter->device, tex_buffer, NULL);
    vkFreeMemory(painter->device, tex_buffer_memory, NULL);
    sdl_result = _painter_generate_mipmaps(painter, &shader->texture_image, VK_FORMAT_R8G8B8A8_SRGB, tex_width, tex_height, shader->mip_levels, SDL_TRUE);
    if (!sdl_result) return SDL_FALSE;
    sdl_result = _painter_create_image_view(painter, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_CUBE, shader->mip_levels, &shader->texture_image, &shader->texture_image_view);
    if (!sdl_result) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create texture image view");
        return SDL_FALSE;
    }

    VkSamplerCreateInfo sampler_create_info;
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.pNext = NULL;
    sampler_create_info.flags = 0;
    sampler_create_info.magFilter = VK_FILTER_LINEAR;
    sampler_create_info.minFilter = VK_FILTER_LINEAR;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_create_info.addressModeU = VK_FILTER_LINEAR;
    sampler_create_info.addressModeV = VK_FILTER_LINEAR;
    sampler_create_info.addressModeW = VK_FILTER_LINEAR;
    sampler_create_info.mipLodBias = 0.0f;
    sampler_create_info.anisotropyEnable = VK_TRUE;
    sampler_create_info.maxAnisotropy = 16.0f;
    sampler_create_info.compareEnable = VK_FALSE;
    sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_create_info.minLod = 0.0f;
    sampler_create_info.maxLod = (float) shader->mip_levels;
    sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_create_info.unnormalizedCoordinates = VK_FALSE;
    result = vkCreateSampler(painter->device, &sampler_create_info, NULL, &shader->texture_sampler);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create texture image sampler");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

extern SDL_bool _painter_load_image_and_sampler(EsPainter* painter, const char* filepath, ShaderData* shader) {
    VkResult result;
    SDL_bool sdl_result;
    int tex_width;
    int tex_height;
    int tex_channels;
    unsigned char* tex_data;
    VkBuffer tex_buffer;
    VkDeviceMemory tex_buffer_memory;
    VkDeviceSize tex_size;
    stbi_uc* pixels;

    pixels = stbi_load(filepath, &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    if (!pixels) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not load texture");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    shader->mip_levels = (Uint32) (SDL_floor(warehouse_log_2(SDL_max((float) tex_width, (float) tex_height))) + 1.0f);
    tex_size = tex_width * tex_height * 4;
    sdl_result = _painter_create_buffer(painter, tex_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &tex_buffer, &tex_buffer_memory);
    if (!sdl_result) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create texture buffer");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    vkMapMemory(painter->device, tex_buffer_memory, 0, tex_size, 0, &tex_data);
    SDL_memcpy(tex_data, pixels, tex_size);
    vkUnmapMemory(painter->device, tex_buffer_memory);
    stbi_image_free(pixels);

    sdl_result = _painter_create_image(painter, tex_width, tex_height, shader->mip_levels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &shader->texture_image, &shader->texture_image_memory, 1, SDL_FALSE);
    if (!sdl_result) return SDL_FALSE;
    sdl_result = _painter_transition_image_layout(painter, &shader->texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, shader->mip_levels, 1);
    if (!sdl_result) return SDL_FALSE;
    sdl_result = _painter_copy_buffer_to_image(painter, &tex_buffer, &shader->texture_image, tex_width, tex_height);
    if (!sdl_result) return SDL_FALSE;
    vkDestroyBuffer(painter->device, tex_buffer, NULL);
    vkFreeMemory(painter->device, tex_buffer_memory, NULL);
    sdl_result = _painter_generate_mipmaps(painter, &shader->texture_image, VK_FORMAT_R8G8B8A8_SRGB, tex_width, tex_height, shader->mip_levels, SDL_FALSE);
    if (!sdl_result) return SDL_FALSE;
    sdl_result = _painter_create_image_view(painter, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, shader->mip_levels, &shader->texture_image, &shader->texture_image_view);
    if (!sdl_result) return SDL_FALSE;

    VkSamplerCreateInfo sampler_create_info;
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.pNext = NULL;
    sampler_create_info.flags = 0;
    sampler_create_info.magFilter = VK_FILTER_LINEAR;
    sampler_create_info.minFilter = VK_FILTER_LINEAR;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_create_info.addressModeU = VK_FILTER_LINEAR;
    sampler_create_info.addressModeV = VK_FILTER_LINEAR;
    sampler_create_info.addressModeW = VK_FILTER_LINEAR;
    sampler_create_info.mipLodBias = 0.0f;
    sampler_create_info.anisotropyEnable = VK_TRUE;
    sampler_create_info.maxAnisotropy = 16.0f;
    sampler_create_info.compareEnable = VK_FALSE;
    sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_create_info.minLod = 0.0f;
    sampler_create_info.maxLod = (float) shader->mip_levels;
    sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_create_info.unnormalizedCoordinates = VK_FALSE;
    result = vkCreateSampler(painter->device, &sampler_create_info, NULL, &shader->texture_sampler);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create texture image sampler");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    return SDL_TRUE;
}


extern SDL_bool _painter_custom_error(const char* header, const char* message) {
    warehouse_error_popup(header, message);
    return SDL_FALSE;
}

extern SDL_bool _painter_cleanup_error(EsPainter* painter, const char* header, const char* message) {
    warehouse_error_popup(header, message);
    painter_cleanup(painter);
    return SDL_FALSE;
}

SDL_bool _painter_swapchain_renderpass_init(EsPainter* painter) {
    VkResult result;
    SDL_bool sdl_result;

    const Uint32 required_device_extensions_count = 1;
    const char** required_device_extensions = (const char**) SDL_malloc(required_device_extensions_count * sizeof(char*));
    required_device_extensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    Uint32 device_extensions_count;
    result = vkEnumerateDeviceExtensionProperties(painter->physical_device, NULL, &device_extensions_count, NULL);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not get available device extensions.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    VkExtensionProperties* device_extensions = (VkExtensionProperties*) SDL_malloc(device_extensions_count * sizeof(VkExtensionProperties));
    result = vkEnumerateDeviceExtensionProperties(painter->physical_device, NULL, &device_extensions_count, device_extensions);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not get available device extensions.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    Uint32 required_device_extensions_available = 0;
    for (Uint32 i=0; i<device_extensions_count; i++) {
        for (Uint32 j=0; j<required_device_extensions_count; j++) {
            if (SDL_strcmp(required_device_extensions[j], device_extensions[i].extensionName) == 0) {
                required_device_extensions_available++;
                break;
            }
        }
    }
    if (required_device_extensions_available != required_device_extensions_count) {
        warehouse_error_popup("Error in Vulkan Setup.", "The required device extensions are not available.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    VkSurfaceCapabilitiesKHR surface_capabilities;
    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(painter->physical_device, painter->surface, &surface_capabilities);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not get physical device surface capabilities.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    Uint32 surface_formats_count;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(painter->physical_device, painter->surface, &surface_formats_count, NULL);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not get physical device surface formats.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    if (surface_formats_count==0) {
        warehouse_error_popup("Error in Vulkan Setup.", "Device does not have surface formats");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    VkSurfaceFormatKHR* surface_formats = (VkSurfaceFormatKHR*) SDL_malloc(surface_formats_count * sizeof(VkSurfaceFormatKHR));
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(painter->physical_device, painter->surface, &surface_formats_count, surface_formats);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not get physical device surface formats.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    Uint32 present_modes_count;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(painter->physical_device, painter->surface, &present_modes_count, NULL);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not get physical device present modes.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    if (present_modes_count==0) {
        warehouse_error_popup("Error in Vulkan Setup.", "Device does not have present modes");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    VkPresentModeKHR* present_modes = (VkPresentModeKHR*) SDL_malloc(present_modes_count * sizeof(VkPresentModeKHR));
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(painter->physical_device, painter->surface, &present_modes_count, present_modes);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not get physical device present modes.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    VkSurfaceFormatKHR selected_surface_format = surface_formats[0];
    for (Uint32 i=0; i<surface_formats_count; i++) {
        if (surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            selected_surface_format = surface_formats[i];
            break;
        }
    }
    VkPresentModeKHR selected_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (Uint32 i=0; i<present_modes_count; i++) {
        if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            selected_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }
    VkExtent2D swap_extent;
    if (surface_capabilities.currentExtent.width != UINT32_MAX)
        swap_extent = surface_capabilities.currentExtent;
    else {
        SDL_Log("Calculating the swap extent.");
        // TODO (17 Oct 2020 sam): Vulkan tutorial does some random fancy shit here
        // See what this should be.
        int width;
        int height;
        SDL_Vulkan_GetDrawableSize(painter->window, &width, &height);
        swap_extent.width = SDL_max(surface_capabilities.minImageExtent.width, SDL_min(surface_capabilities.maxImageExtent.width, (Uint32)width));
        swap_extent.height = SDL_max(surface_capabilities.minImageExtent.height, SDL_min(surface_capabilities.maxImageExtent.height, (Uint32)height));
    }

    Uint32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(painter->physical_device, &queue_family_count, NULL);
    SDL_Log("Found %i queue families.\n", queue_family_count);
    VkQueueFamilyProperties* queue_families = (VkQueueFamilyProperties*) SDL_malloc(queue_family_count * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(painter->physical_device, &queue_family_count, queue_families);
    int graphics_queue_family = -1;
    int presentation_queue_family = -1;
    for (Uint32 i=0; i<queue_family_count; i++) {
        if (presentation_queue_family < 0) {
            VkBool32 supports_presentaion;
            result = vkGetPhysicalDeviceSurfaceSupportKHR(painter->physical_device, i, painter->surface, &supports_presentaion);
            if (result != VK_SUCCESS) {
                warehouse_error_popup("Error in Vulkan Setup.", "Could not check queue family presentation support.");
                painter_cleanup(painter);
                return SDL_FALSE;
            }
            if (supports_presentaion)
                presentation_queue_family = i;
        }
        if (graphics_queue_family < 0) {
            if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                graphics_queue_family = i;
        }
        if (presentation_queue_family >= 0 && graphics_queue_family >= 0)
            break;
    }
    if (graphics_queue_family < 0) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not find queue family with graphics");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    if (presentation_queue_family < 0) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not find queue family with presentaition support");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    // TODO (17 Oct 2020 sam): Currently, we're assuming the same queue family for both.
    // If this is not the case, we would have to update the queue creation that comes next.
    if (graphics_queue_family != presentation_queue_family) {
        warehouse_error_popup("Error in Vulkan Setup.", "Currently do not support different presentation and grahics queue families");
        painter_cleanup(painter);
        return SDL_FALSE;
    }

#if DEBUG_BUILD==SDL_TRUE
    const Uint32 validation_layers_count = 1;
    const char** validation_layers = (const char**) SDL_malloc(validation_layers_count * sizeof(char*));
    validation_layers[0] =  "VK_LAYER_KHRONOS_validation";
    Uint32 layer_count;
    result = vkEnumerateInstanceLayerProperties(&layer_count, NULL);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "There was an error setting up Vulkan. Could not get instance layer properties");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    VkLayerProperties* layer_properties = (VkLayerProperties*) SDL_malloc(layer_count * sizeof(VkLayerProperties));
    result = vkEnumerateInstanceLayerProperties(&layer_count, layer_properties);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "There was an error setting up Vulkan. Could not get instance layer properties");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    SDL_bool validation_available = SDL_FALSE;
    for (Uint32 i=0; i<layer_count; i++) {
        for (Uint32 j=0; j<validation_layers_count; j++) {
            if (SDL_strcmp(validation_layers[j], layer_properties[i].layerName) == 0) {
                validation_available = SDL_TRUE;
                break;
            }
        }
        if (validation_available)
            break;
    }
    if (!validation_available) {
        warehouse_error_popup("Error in Vulkan Setup.", "Validation layer not available.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    SDL_free(layer_properties);
    SDL_Log("yes debug build\n");
#endif
    
    VkDeviceQueueCreateInfo queue_create_info;
    float graphics_queue_priority = 1.0f;
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.pNext = NULL;
    queue_create_info.flags = 0;
    queue_create_info.queueFamilyIndex = graphics_queue_family;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &graphics_queue_priority;
    // TODO (16 Oct 2020 sam): Figure out whether all features here are correctly
    // being set to SDL_FALSE.
    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(painter->physical_device, &device_features);
    if (!device_features.samplerAnisotropy) {
        warehouse_error_popup("Error in Vulkan Setup.", "GPU does not support sampler anisotropy.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    VkDeviceCreateInfo device_create_info;
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = NULL;
    device_create_info.flags = 0;
    device_create_info.queueCreateInfoCount = 1;
#if DEBUG_BUILD==SDL_TRUE
    device_create_info.enabledLayerCount = validation_layers_count;
    device_create_info.ppEnabledLayerNames = validation_layers;
#else
    device_create_info.enabledLayerCount = 0;
#endif
    device_create_info.enabledExtensionCount = required_device_extensions_count;
    device_create_info.ppEnabledExtensionNames = required_device_extensions;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.pEnabledFeatures = &device_features;
    result = vkCreateDevice(painter->physical_device, &device_create_info, NULL, &painter->device);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create device.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    Uint32 graphics_queue_index = 0;
    Uint32 presentation_queue_index = 0;
    vkGetDeviceQueue(painter->device, graphics_queue_family, graphics_queue_index, &painter->graphics_queue);
    vkGetDeviceQueue(painter->device, presentation_queue_family, presentation_queue_index, &painter->presentation_queue);

    Uint32 queue_family_indices[2];
    queue_family_indices[0] = graphics_queue_family;
    queue_family_indices[1] = presentation_queue_family;
    Uint32 swapchain_image_count = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount>0 && swapchain_image_count>surface_capabilities.maxImageCount)
        swapchain_image_count = surface_capabilities.maxImageCount;
    SDL_Log("Image Count: %i\n", swapchain_image_count);
    VkSwapchainCreateInfoKHR swapchain_create_info;
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.pNext = NULL;
    swapchain_create_info.flags = 0;
    swapchain_create_info.surface = painter->surface;
    swapchain_create_info.minImageCount = swapchain_image_count;
    swapchain_create_info.imageFormat = selected_surface_format.format;
    swapchain_create_info.imageColorSpace = selected_surface_format.colorSpace;
    swapchain_create_info.imageExtent = swap_extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (graphics_queue_family != presentation_queue_family) {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0;
        swapchain_create_info.pQueueFamilyIndices = NULL;
    }
    swapchain_create_info.preTransform = surface_capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = selected_present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;
    result = vkCreateSwapchainKHR(painter->device, &swapchain_create_info, NULL, &painter->swapchain);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create swapchain.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }

    result = vkGetSwapchainImagesKHR(painter->device, painter->swapchain, &painter->swapchain_image_count, NULL);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not get swapchain images.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    VkImage* swapchain_images = (VkImage*) SDL_malloc(painter->swapchain_image_count * sizeof(VkImage));
    result = vkGetSwapchainImagesKHR(painter->device, painter->swapchain, &painter->swapchain_image_count, swapchain_images);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not get painter->swapchain images.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    painter->swapchain_image_format = swapchain_create_info.imageFormat;
    painter->swapchain_extent = swapchain_create_info.imageExtent;
    painter->swapchain_image_views = (VkImageView*) SDL_malloc(painter->swapchain_image_count * sizeof(VkImageView));
    for (Uint32 i=0; i<painter->swapchain_image_count; i++) {
        VkImageViewCreateInfo imageview_create_info;
        imageview_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageview_create_info.pNext = NULL;
        imageview_create_info.flags = 0;
        imageview_create_info.image = swapchain_images[i];
        imageview_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageview_create_info.format = painter->swapchain_image_format;
        imageview_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageview_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageview_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageview_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageview_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageview_create_info.subresourceRange.baseMipLevel = 0;
        imageview_create_info.subresourceRange.levelCount = 1;
        imageview_create_info.subresourceRange.baseArrayLayer = 0;
        imageview_create_info.subresourceRange.layerCount = 1;
        result = vkCreateImageView(painter->device, &imageview_create_info, NULL, &painter->swapchain_image_views[i]);
        if (result != VK_SUCCESS) {
            warehouse_error_popup("Error in Vulkan Setup.", "Could not create imageview.");
            painter_cleanup(painter);
            return SDL_FALSE;
        }
    }

    VkCommandPoolCreateInfo command_pool_create_info;
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.pNext = NULL;
    command_pool_create_info.flags = 0;
    command_pool_create_info.queueFamilyIndex = graphics_queue_family;
    result = vkCreateCommandPool(painter->device, &command_pool_create_info, NULL, &painter->command_pool);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create command_pool");
        painter_cleanup(painter);
        return SDL_FALSE;
    }

    // Find Supported Format for Depth Buffer
    // TODO (30 Oct 2020 sam): The depth_buffer stuff needs to be recreated on window resize
    VkFormat desired_formats[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    VkFormat depth_buffer_format = VK_FORMAT_UNDEFINED;
    VkImageTiling depth_image_tiling = VK_IMAGE_TILING_OPTIMAL;
    VkFormatFeatureFlags depth_feature_flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for (Uint32 i=0; i<3; i++) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(painter->physical_device, desired_formats[i], &properties);
        if (depth_image_tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & depth_feature_flags) == depth_feature_flags) {
            depth_buffer_format = desired_formats[i];
            break;
        }
    }
    if (depth_buffer_format == VK_FORMAT_UNDEFINED) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not find suitable depth buffer format");
        painter_cleanup(painter);
        return SDL_FALSE;
    }

    VkAttachmentDescription color_attachment;
    color_attachment.flags = 0;
    color_attachment.format = painter->swapchain_image_format;
    color_attachment.samples = painter->msaa_samples;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkAttachmentDescription depth_attachment;
    depth_attachment.flags = 0;
    depth_attachment.format = depth_buffer_format;
    depth_attachment.samples = painter->msaa_samples;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentDescription color_resolve_attachment;
    color_resolve_attachment.flags = 0;
    color_resolve_attachment.format = painter->swapchain_image_format;
    color_resolve_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_resolve_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_resolve_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_resolve_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_resolve_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_resolve_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_resolve_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference color_attachment_reference;
    color_attachment_reference.attachment = 0;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkAttachmentReference depth_attachment_reference;
    depth_attachment_reference.attachment = 1;
    depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentReference color_resolve_attachment_reference;
    color_resolve_attachment_reference.attachment = 2;
    color_resolve_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass;
    subpass.flags = 0;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_reference;
    subpass.pResolveAttachments = &color_resolve_attachment_reference;
    subpass.pDepthStencilAttachment = &depth_attachment_reference;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;
    VkSubpassDependency subpass_dependency;
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpass_dependency.dependencyFlags= 0;
    VkAttachmentDescription attachments[3];
    attachments[0] = color_attachment;
    attachments[1] = depth_attachment;
    attachments[2] = color_resolve_attachment;
    VkRenderPassCreateInfo render_pass_create_info;
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.pNext = NULL;
    render_pass_create_info.flags = 0;
    render_pass_create_info.attachmentCount = 3;
    render_pass_create_info.pAttachments = attachments;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass;
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies = &subpass_dependency;
    result = vkCreateRenderPass(painter->device, &render_pass_create_info, NULL, &painter->render_pass);
    if (result != VK_SUCCESS) return _painter_cleanup_error(painter, "Vulkan Setup Error", "Could not create renderpass");

    sdl_result = _painter_create_image(painter, painter->swapchain_extent.width, painter->swapchain_extent.height, 1, painter->msaa_samples, painter->swapchain_image_format, VK_IMAGE_TILING_OPTIMAL,  VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &painter->color_image, &painter->color_image_memory, 1, SDL_FALSE);
    if (!sdl_result) return _painter_custom_error("Setup Error", "color_image");
    sdl_result = _painter_create_image_view(painter, painter->swapchain_image_format, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 1, &painter->color_image, &painter->color_image_view);
    if (!sdl_result) return _painter_custom_error("Setup Error", "color_image_view");

    sdl_result = _painter_create_image(painter, painter->swapchain_extent.width, painter->swapchain_extent.height, 1, painter->msaa_samples, depth_buffer_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &painter->depth_image, &painter->depth_image_memory, 1, SDL_FALSE);
    if (!sdl_result) return _painter_custom_error("Setup Error", "depth_image");
    sdl_result = _painter_create_image_view(painter, depth_buffer_format, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D, 1, &painter->depth_image, &painter->depth_image_view);
    if (!sdl_result) return _painter_custom_error("Setup Error", "depth_image_view");
    sdl_result = _painter_transition_image_layout(painter, &painter->depth_image, depth_buffer_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, 1);
    if (!sdl_result) return SDL_FALSE;

    painter->swapchain_framebuffers = (VkFramebuffer*) SDL_malloc(painter->swapchain_image_count * sizeof(VkFramebuffer));
    for (Uint32 i=0; i<painter->swapchain_image_count; i++) {
        VkImageView image_view_attachments[3];
        image_view_attachments[0] = painter->color_image_view;
        image_view_attachments[1] = painter->depth_image_view;
        image_view_attachments[2] = painter->swapchain_image_views[i];
        VkFramebufferCreateInfo framebuffer_create_info;
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.pNext = NULL;
        framebuffer_create_info.flags = 0;
        framebuffer_create_info.renderPass = painter->render_pass;
        framebuffer_create_info.attachmentCount = 3;
        framebuffer_create_info.pAttachments = image_view_attachments;
        framebuffer_create_info.width = painter->swapchain_extent.width;
        framebuffer_create_info.height = painter->swapchain_extent.height;
        framebuffer_create_info.layers = 1;
        result = vkCreateFramebuffer(painter->device, &framebuffer_create_info, NULL, &painter->swapchain_framebuffers[i]);
        if (result != VK_SUCCESS) {
            warehouse_error_popup("Error in Vulkan Setup.", "Could not create framebuffer");
            painter_cleanup(painter);
            return SDL_FALSE;
        }
    }
#if DEBUG_BUILD==SDL_TRUE
    SDL_free((char*)validation_layers);
#endif
    SDL_free(present_modes);
    SDL_free(surface_formats);
    SDL_free(queue_families);
    SDL_free((char*)required_device_extensions);
    SDL_free(swapchain_images);
    SDL_free(device_extensions);

    return SDL_TRUE;
}

void _painter_read_obj_file(const char* filename, const int is_mtl, const char *obj_filename, char** data, size_t* len) {
    obj_filename;
    if (is_mtl == 1) {
        SDL_Log("We currently don't load mtl files\n");
        *data = NULL;
        *len = 0;
        return;
    }
    SDL_RWops* obj_file;
    obj_file = SDL_RWFromFile(filename, "rb");
    Sint64 file_size;
    size_t read_size;
    Sint64 result;
    if (obj_file == NULL) {
        *data = NULL;
        *len = 0;
        return;
    }
    file_size = SDL_RWseek(obj_file, 0, RW_SEEK_END);
    if (file_size < 0) {
        SDL_RWclose(obj_file);
        *data = NULL;
        *len = 0;
        return;
    }
    result = SDL_RWseek(obj_file, 0, RW_SEEK_SET);
    if (result < 0) {
        SDL_RWclose(obj_file);
        *data = NULL;
        *len = 0;
        return;
    }
    char* temp_buffer = (char*) SDL_malloc(file_size * sizeof(char));
    read_size = SDL_RWread(obj_file, temp_buffer, sizeof(char), file_size);
    if ((Sint64)(read_size*sizeof(char)) != file_size) {
        SDL_free(temp_buffer);
        SDL_RWclose(obj_file);
        *data = NULL;
        *len = 0;
        return;
    }
    SDL_RWclose(obj_file);
    *data = temp_buffer;
    *len = read_size;
    return;
}

SDL_bool _painter_initialise_sdl_window(EsPainter* painter, const char* window_name) {
    int init_success = SDL_Init(SDL_INIT_VIDEO);
    if (init_success !=0) {
        warehouse_error_popup("Error in SDL Initialisation.", SDL_GetError());
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    SDL_Window* window;
    window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              1024, 768, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    if (window == NULL) {
        warehouse_error_popup("Error in SDL Window Creation.", SDL_GetError());
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    painter->window = window;
    painter->start_time = SDL_GetTicks();


    return SDL_TRUE;
}

SDL_bool _painter_init_instance(EsPainter* painter) {
    VkResult result;
    SDL_bool sdl_result;

    VkApplicationInfo app_info;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = NULL;
    app_info.pApplicationName = "Easel";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "chapliboy engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    // TODO (09 Dec 2020 sam): Check whether version 1.1 is available and use that
    app_info.apiVersion = VK_API_VERSION_1_0;

#if DEBUG_BUILD==SDL_TRUE
    const Uint32 validation_layers_count = 1;
    const char** validation_layers = (const char**) SDL_malloc(validation_layers_count * sizeof(char*));
    validation_layers[0] =  "VK_LAYER_KHRONOS_validation";
    Uint32 layer_count;
    result = vkEnumerateInstanceLayerProperties(&layer_count, NULL);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "There was an error setting up Vulkan. Could not get instance layer properties");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    VkLayerProperties* layer_properties = (VkLayerProperties*) SDL_malloc(layer_count * sizeof(VkLayerProperties));
    result = vkEnumerateInstanceLayerProperties(&layer_count, layer_properties);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "There was an error setting up Vulkan. Could not get instance layer properties");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    SDL_bool validation_available = SDL_FALSE;
    for (Uint32 i=0; i<layer_count; i++) {
        for (Uint32 j=0; j<validation_layers_count; j++) {
            if (SDL_strcmp(validation_layers[j], layer_properties[i].layerName) == 0) {
                validation_available = SDL_TRUE;
                break;
            }
        }
        if (validation_available)
            break;
    }
    if (!validation_available) {
        warehouse_error_popup("Error in Vulkan Setup.", "Validation layer not available.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    SDL_free(layer_properties);
    SDL_Log("yes debug build\n");
#endif

    VkInstanceCreateInfo instance_create_info;
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pNext = NULL;
    instance_create_info.pApplicationInfo = &app_info;
    Uint32 required_extensions_count = 0;
    const char** required_extensions;
    sdl_result = SDL_Vulkan_GetInstanceExtensions(painter->window, &required_extensions_count, NULL);
    if (!sdl_result) {
        warehouse_error_popup("Error in getting Required Vulkan Extensions", SDL_GetError());
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    required_extensions = (const char**) SDL_malloc(required_extensions_count * sizeof(char*));
    sdl_result = SDL_Vulkan_GetInstanceExtensions(painter->window, &required_extensions_count, required_extensions);
    if (!sdl_result) {
        warehouse_error_popup("Error in getting Required Vulkan Extensions", SDL_GetError());
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    Uint32 available_extensions_count;
    result = vkEnumerateInstanceExtensionProperties(NULL, &available_extensions_count, NULL);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not get available instance extensions.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    VkExtensionProperties* available_extensions = (VkExtensionProperties*) SDL_malloc(available_extensions_count * sizeof(VkExtensionProperties));
    result = vkEnumerateInstanceExtensionProperties(NULL, &available_extensions_count, available_extensions);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not get available instance extensions.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    Uint32 required_extensions_available = 0;
    for (Uint32 i=0; i<available_extensions_count; i++) {
        for (Uint32 j=0; j<required_extensions_count; j++) {
            if (SDL_strcmp(required_extensions[j], available_extensions[i].extensionName) == 0) {
                required_extensions_available++;
                break;
            }
        }
    }
    if (required_extensions_available != required_extensions_count) {
        warehouse_error_popup("Error in Vulkan Setup.", "The required extensions are not available.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    instance_create_info.enabledExtensionCount = required_extensions_count;
    instance_create_info.ppEnabledExtensionNames = required_extensions;
    instance_create_info.flags = 0;
#if DEBUG_BUILD==SDL_TRUE
    instance_create_info.enabledLayerCount = validation_layers_count;
    instance_create_info.ppEnabledLayerNames = validation_layers;
#else
    instance_create_info.enabledLayerCount = 0;
#endif
    SDL_Log("Creating Vulkan Instance\n");
    result = vkCreateInstance(&instance_create_info, NULL, &painter->instance);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create vulkan instance.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }

    sdl_result = SDL_Vulkan_CreateSurface(painter->window, painter->instance, &painter->surface);
    if (!sdl_result) {
        warehouse_error_popup("Error in getting Creating SDL Vulkan Surface", SDL_GetError());
        painter_cleanup(painter);
        return SDL_FALSE;
    }

#if DEBUG_BUILD==SDL_TRUE
    SDL_free((char*)validation_layers);
#endif
    SDL_free((char*)required_extensions);
    SDL_free(available_extensions);

    return SDL_TRUE;
}

SDL_bool _painter_select_physical_device(EsPainter* painter) {
    VkResult result;

    Uint32 device_count = 0;
    result = vkEnumeratePhysicalDevices(painter->instance, &device_count, NULL);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not get physical devices.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    if (device_count == 0) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not find a GPU with Vulkan support.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    SDL_Log("Found %i Vulkan Devices\n", device_count);
    VkPhysicalDevice* devices = (VkPhysicalDevice*) SDL_malloc(device_count * sizeof(VkPhysicalDevice));
    result = vkEnumeratePhysicalDevices(painter->instance, &device_count, devices);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not get physical devices.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    if (device_count == 1)
        // Since there is only one device available, we select that. Later on, we check
        // whether it has all the functionality that we need.
        painter->physical_device = devices[0];
    else {
        // TODO (16 Oct 2020 sam): Implement selection of best device
        // Would need to check queue families available etc.
        // Just defaulting to first one for now.
        painter->physical_device = devices[0];
    }
    if (painter->physical_device == VK_NULL_HANDLE) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not find a suitable GPU.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    VkPhysicalDeviceProperties physical_device_properties;
    vkGetPhysicalDeviceProperties(painter->physical_device, &physical_device_properties);
    VkSampleCountFlags counts = physical_device_properties.limits.framebufferColorSampleCounts & physical_device_properties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { painter->msaa_samples = VK_SAMPLE_COUNT_64_BIT; }
    else if (counts & VK_SAMPLE_COUNT_32_BIT) { painter->msaa_samples = VK_SAMPLE_COUNT_32_BIT; }
    else if (counts & VK_SAMPLE_COUNT_16_BIT) { painter->msaa_samples = VK_SAMPLE_COUNT_16_BIT; }
    else if (counts & VK_SAMPLE_COUNT_8_BIT) { painter->msaa_samples = VK_SAMPLE_COUNT_8_BIT; }
    else if (counts & VK_SAMPLE_COUNT_4_BIT) { painter->msaa_samples = VK_SAMPLE_COUNT_4_BIT; }
    else if (counts & VK_SAMPLE_COUNT_2_BIT) { painter->msaa_samples = VK_SAMPLE_COUNT_2_BIT; }
    else painter->msaa_samples = VK_SAMPLE_COUNT_1_BIT;
    SDL_free(devices);
    return SDL_TRUE;
}

SDL_bool _painter_create_synchronisation_elements(EsPainter* painter) {
    VkResult result;
    SDL_Log("Creating Synchronisation Elements\n");
    // need to malloc here =P
    painter->image_available_semaphores = (VkSemaphore*) SDL_malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkSemaphore));
    painter->render_finished_semaphores = (VkSemaphore*) SDL_malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkSemaphore));
    VkSemaphoreCreateInfo semaphore_create_info;
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_create_info.pNext = NULL;
    semaphore_create_info.flags = 0;
    painter->in_flight_fences = (VkFence*) SDL_malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkFence));
    painter->images_in_flight = (VkFence*) SDL_calloc(painter->swapchain_image_count, sizeof(VkFence));
    VkFenceCreateInfo fence_create_info;
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.pNext = NULL;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (Uint32 i=0; i<MAX_FRAMES_IN_FLIGHT; i++) {
        result = vkCreateSemaphore(painter->device, &semaphore_create_info, NULL, &painter->image_available_semaphores[i]);
        if (result != VK_SUCCESS) {
            warehouse_error_popup("Error in Vulkan Setup.", "Could not create image semaphore");
            painter_cleanup(painter);
            return SDL_FALSE;
        }
        result = vkCreateSemaphore(painter->device, &semaphore_create_info, NULL, &painter->render_finished_semaphores[i]);
        if (result != VK_SUCCESS) {
            warehouse_error_popup("Error in Vulkan Setup.", "Could not create image semaphore");
            painter_cleanup(painter);
            return SDL_FALSE;
        }
        result = vkCreateFence(painter->device, &fence_create_info, NULL, &painter->in_flight_fences[i]);
        if (result != VK_SUCCESS) {
            warehouse_error_popup("Error in Vulkan Setup.", "Could not create fence");
            painter_cleanup(painter);
            return SDL_FALSE;
        }
    }
    return SDL_TRUE; 
}

SDL_bool _painter_load_buffer_via_staging(EsPainter* painter, void* data, VkDeviceMemory* memory, VkBuffer* src, VkBuffer* dst, Uint32 size) {
    VkResult result;
    void* staging_data;
    result = vkMapMemory(painter->device, *memory, 0, size, 0, &staging_data);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Setup.", "Could not map staging buffer memory");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    SDL_memcpy(staging_data, data, size);
    vkUnmapMemory(painter->device, *memory);
    VkBufferCopy buffer_copy_info;
    buffer_copy_info.srcOffset = 0;
    buffer_copy_info.dstOffset = 0;
    buffer_copy_info.size = size;
    VkCommandBuffer command_buffer = _painter_begin_single_use_command_buffer(painter);
    vkCmdCopyBuffer(command_buffer, *src, *dst, 1, &buffer_copy_info);
    _painter_end_single_use_command_buffer(painter, &command_buffer);
    return SDL_TRUE;
}

void _painter_cleanup_swapchain(EsPainter* painter) {
    vkQueueWaitIdle(painter->presentation_queue);
    vkDeviceWaitIdle(painter->device);
    _painter_shader_cleanup(painter, painter->skybox_shader);
    for (Uint32 i=0; i<painter->num_shaders; i++) {
        _painter_shader_cleanup(painter, &painter->shaders[i]);
    }
    if (painter->swapchain_framebuffers) {
        for (Uint32 i=0; i<painter->swapchain_image_count; i++)
            vkDestroyFramebuffer(painter->device, painter->swapchain_framebuffers[i], NULL);
    }
    if (painter->command_buffers)
        vkFreeCommandBuffers(painter->device, painter->command_pool, painter->swapchain_image_count, painter->command_buffers);
    if (painter->color_image)
        vkDestroyImage(painter->device, painter->color_image, NULL);
    if (painter->color_image_view)
        vkDestroyImageView(painter->device, painter->color_image_view, NULL);
    if (painter->color_image_memory)
        vkFreeMemory(painter->device, painter->color_image_memory, NULL);
    if (painter->depth_image_view)
        vkDestroyImageView(painter->device, painter->depth_image_view, NULL);
    if (painter->depth_image)
        vkDestroyImage(painter->device, painter->depth_image, NULL);
    if (painter->depth_image_memory)
        vkFreeMemory(painter->device, painter->depth_image_memory, NULL);
    if (painter->render_pass)
        vkDestroyRenderPass(painter->device, painter->render_pass, NULL);
    if (painter->swapchain_image_count > 0) {
        for (Uint32 i=0; i<painter->swapchain_image_count; i++)
            vkDestroyImageView(painter->device, painter->swapchain_image_views[i], NULL);
    }
    if (painter->swapchain)
        vkDestroySwapchainKHR(painter->device, painter->swapchain, NULL);
    return;
}

void _painter_shader_cleanup(EsPainter* painter, ShaderData* shader) {
    if (shader->texture_sampler)
        vkDestroySampler(painter->device, shader->texture_sampler, NULL);
    if (shader->texture_image_view)
        vkDestroyImageView(painter->device, shader->texture_image_view, NULL);
    if (shader->texture_image)
        vkDestroyImage(painter->device, shader->texture_image, NULL);
    if (shader->texture_image_memory)
        vkFreeMemory(painter->device, shader->texture_image_memory, NULL);
    if (shader->descriptor_pool)
        vkDestroyDescriptorPool(painter->device, shader->descriptor_pool, NULL);
    if (shader->index_buffer)
        vkDestroyBuffer(painter->device, shader->index_buffer, NULL);
    if (shader->index_buffer_memory)
        vkFreeMemory(painter->device, shader->index_buffer_memory, NULL);
    if (shader->index_staging_buffer)
        vkDestroyBuffer(painter->device, shader->index_staging_buffer, NULL);
    if (shader->index_staging_buffer_memory)
        vkFreeMemory(painter->device, shader->index_staging_buffer_memory, NULL);
    if (shader->vertex_buffer)
        vkDestroyBuffer(painter->device, shader->vertex_buffer, NULL);
    if (shader->vertex_buffer_memory)
        vkFreeMemory(painter->device, shader->vertex_buffer_memory, NULL);
    if (shader->vertex_staging_buffer)
        vkDestroyBuffer(painter->device, shader->vertex_staging_buffer, NULL);
    if (shader->vertex_staging_buffer_memory)
        vkFreeMemory(painter->device, shader->vertex_staging_buffer_memory, NULL);
    if (shader->vertex_shader_module)
        vkDestroyShaderModule(painter->device, shader->vertex_shader_module, NULL);
    if (shader->fragment_shader_module)
        vkDestroyShaderModule(painter->device, shader->fragment_shader_module, NULL);
    if (shader->pipeline)
        vkDestroyPipeline(painter->device, shader->pipeline, NULL);
    if (shader->descriptor_set_layout)
        vkDestroyDescriptorSetLayout(painter->device, shader->descriptor_set_layout, NULL);
    if (shader->pipeline_layout)
        vkDestroyPipelineLayout(painter->device, shader->pipeline_layout, NULL);
}

void painter_cleanup(EsPainter* painter) {
    _painter_cleanup_swapchain(painter);
    if (painter->uniform_buffers)
        for (Uint32 i=0; i<painter->swapchain_image_count; i++)
            vkDestroyBuffer(painter->device, painter->uniform_buffers[i], NULL);
    if (painter->uniform_buffers_memory)
        for (Uint32 i=0; i<painter->swapchain_image_count; i++)
            vkFreeMemory(painter->device, painter->uniform_buffers_memory[i], NULL);
    if (painter->in_flight_fences)
        for (Uint32 i=0; i<MAX_FRAMES_IN_FLIGHT; i++)
            vkDestroyFence(painter->device, painter->in_flight_fences[i], NULL);
    if (painter->image_available_semaphores)
        for (Uint32 i=0; i<MAX_FRAMES_IN_FLIGHT; i++)
            vkDestroySemaphore(painter->device, painter->image_available_semaphores[i], NULL);
    if (painter->render_finished_semaphores)
        for (Uint32 i=0; i<MAX_FRAMES_IN_FLIGHT; i++)
            vkDestroySemaphore(painter->device, painter->render_finished_semaphores[i], NULL);
    if (painter->command_pool)
        vkDestroyCommandPool(painter->device, painter->command_pool, NULL);
    if (painter->device) {
        vkDeviceWaitIdle(painter->device);
        vkDestroyDevice(painter->device, NULL);
    }
    if (painter->surface)
        vkDestroySurfaceKHR(painter->instance, painter->surface, NULL);
    if (painter->instance)
        vkDestroyInstance(painter->instance, NULL);
    if (painter->window)
        SDL_DestroyWindow(painter->window);
    SDL_free(painter->command_buffers);
    SDL_free(painter->swapchain_image_views);
    SDL_free(painter->swapchain_framebuffers);
    SDL_free(painter->uniform_buffers);
    SDL_free(painter->uniform_buffers_memory);
    SDL_Quit();
}

Sint64 painter_read_shader_file(const char* filename, Uint32** buffer) {
    SDL_RWops* shader_file;
    shader_file = SDL_RWFromFile(filename, "rb");
    Sint64 file_size;
    size_t read_size;
    Sint64 result;
    if (shader_file == NULL)
        return -1;
    file_size = SDL_RWseek(shader_file, 0, RW_SEEK_END);
    if (file_size < 0) {
        SDL_RWclose(shader_file);
        return -2;
    }
    result = SDL_RWseek(shader_file, 0, RW_SEEK_SET);
    if (result < 0) {
        SDL_RWclose(shader_file);
        return -4;
    }
    Uint32* temp_buffer = (Uint32*) SDL_malloc(file_size * sizeof(Uint32));
    read_size = SDL_RWread(shader_file, temp_buffer, sizeof(Uint32), file_size);
    if ((Sint64)(read_size*sizeof(Uint32)) != file_size) {
        SDL_free(temp_buffer);
        SDL_RWclose(shader_file);
        return -5;
    }
    SDL_RWclose(shader_file);
    *buffer = temp_buffer;
    return file_size;
}

SDL_bool _painter_recreate_swapchain(EsPainter* painter) {
    _painter_cleanup_swapchain(painter);
    SDL_bool sdl_result = _painter_create_swapchain(painter);
    if (!sdl_result) {
        warehouse_error_popup("Error in Rendering.", "Could not recreate swapchain");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

SDL_bool _painter_create_buffer(EsPainter* painter, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags property_flags, VkBuffer* buffer, VkDeviceMemory* buffer_memory) {
    VkResult result;
    VkBufferCreateInfo buffer_create_info;
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.pNext = NULL;
    buffer_create_info.flags = 0;
    buffer_create_info.size = size;
    buffer_create_info.usage = usage;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_create_info.queueFamilyIndexCount = 0;
    buffer_create_info.pQueueFamilyIndices = NULL;
    result = vkCreateBuffer(painter->device, &buffer_create_info, NULL, buffer);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create vertex buffer");
        painter_cleanup(painter);
        return SDL_FALSE;
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(painter->device, *buffer, &memory_requirements);
    Uint32 memory_type_index = _painter_find_memory_type(painter, property_flags, &memory_requirements);
    if (memory_type_index == UINT32_MAX) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not find suitable memory");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    VkMemoryAllocateInfo memory_allocation_info;
    memory_allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocation_info.pNext = NULL;
    memory_allocation_info.allocationSize = memory_requirements.size;
    memory_allocation_info.memoryTypeIndex = memory_type_index;
    result = vkAllocateMemory(painter->device, &memory_allocation_info, NULL, buffer_memory);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not allocate vertex buffer memory");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    result = vkBindBufferMemory(painter->device, *buffer, *buffer_memory, 0);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not bind vertex buffer memory");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

Uint32 _painter_find_memory_type(EsPainter* painter, VkMemoryPropertyFlags property_flags, VkMemoryRequirements* memory_requirements) {
    Uint32 memory_type_index = UINT32_MAX;
    VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
    vkGetPhysicalDeviceMemoryProperties(painter->physical_device, &physical_device_memory_properties);
    for (Uint32 i=0; i<physical_device_memory_properties.memoryTypeCount; i++) {
        SDL_bool memory_is_suitable = memory_requirements->memoryTypeBits & (1<<i);
        SDL_bool memory_has_properties = (physical_device_memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags; 
        if (memory_is_suitable && memory_has_properties) {
            memory_type_index = i;
            break;
        }
    }
    return memory_type_index;
}

SDL_bool _painter_transition_image_layout(EsPainter* painter, VkImage* image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, Uint32 mip_levels, Uint32 layer_count) {
    SDL_bool sdl_result;
    VkCommandBuffer command_buffer = _painter_begin_single_use_command_buffer(painter);
    if (command_buffer == NULL) return SDL_FALSE;
    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags dest_stage;
    VkImageMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = NULL;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = *image;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mip_levels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layer_count;
    if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dest_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dest_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dest_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not handle transition layouts");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    vkCmdPipelineBarrier(command_buffer, source_stage, dest_stage, 0, 0, NULL, 0, NULL, 1, &barrier);
    sdl_result = _painter_end_single_use_command_buffer(painter, &command_buffer);
    if (!sdl_result) return SDL_FALSE;
    return SDL_TRUE;
}

SDL_bool _painter_copy_buffer_to_image(EsPainter* painter, VkBuffer* buffer, VkImage* image, Uint32 width, Uint32 height) {
    SDL_bool sdl_result;
    VkCommandBuffer command_buffer = _painter_begin_single_use_command_buffer(painter);
    if (command_buffer == NULL) return SDL_FALSE;
    VkBufferImageCopy image_region;
    image_region.bufferOffset = 0;
    image_region.bufferRowLength = 0;
    image_region.bufferImageHeight = 0;
    image_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_region.imageSubresource.mipLevel = 0;
    image_region.imageSubresource.baseArrayLayer = 0;
    image_region.imageSubresource.layerCount = 1;
    image_region.imageOffset.x = 0;
    image_region.imageOffset.y = 0;
    image_region.imageOffset.z = 0;
    image_region.imageExtent.width = width;
    image_region.imageExtent.height = height;
    image_region.imageExtent.depth = 1;
    vkCmdCopyBufferToImage(command_buffer, *buffer, *image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_region);
    sdl_result = _painter_end_single_use_command_buffer(painter, &command_buffer);
    if (!sdl_result) return SDL_FALSE;
    return SDL_TRUE;
}

SDL_bool _painter_copy_cubemap_buffer_to_image(EsPainter* painter, VkBuffer* buffer, VkImage* image, Uint32 width, Uint32 height) {
    SDL_bool sdl_result;
    VkCommandBuffer command_buffer = _painter_begin_single_use_command_buffer(painter);
    if (command_buffer == NULL) return SDL_FALSE;
    VkBufferImageCopy image_regions[6];
    for (Uint32 i=0; i<6; i++) {
        image_regions[i].bufferOffset = width * height * 4 * i;
        image_regions[i].bufferRowLength = 0;
        image_regions[i].bufferImageHeight = 0;
        image_regions[i].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_regions[i].imageSubresource.mipLevel = 0;
        image_regions[i].imageSubresource.baseArrayLayer = i;
        image_regions[i].imageSubresource.layerCount = 1;
        image_regions[i].imageOffset.x = 0;
        image_regions[i].imageOffset.y = 0;
        image_regions[i].imageOffset.z = 0;
        image_regions[i].imageExtent.width = width;
        image_regions[i].imageExtent.height = height;
        image_regions[i].imageExtent.depth = 1;
    }
    vkCmdCopyBufferToImage(command_buffer, *buffer, *image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6, image_regions);
    sdl_result = _painter_end_single_use_command_buffer(painter, &command_buffer);
    if (!sdl_result) return SDL_FALSE;
    return SDL_TRUE;
}

SDL_bool _painter_create_image(EsPainter* painter, Uint32 width, Uint32 height, Uint32 mip_levels, VkSampleCountFlagBits num_samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* image_memory, Uint32 array_layers, SDL_bool is_cube) {
    VkResult result;
    VkImageCreateInfo image_create_info;
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    if (is_cube) 
        image_create_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    else
        image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = format;
    image_create_info.extent.depth = 1;
    image_create_info.extent.width = width;
    image_create_info.extent.height = height;
    image_create_info.mipLevels = mip_levels;
    image_create_info.arrayLayers = array_layers;
    image_create_info.samples = num_samples;
    image_create_info.tiling = tiling;
    image_create_info.usage = usage;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    result = vkCreateImage(painter->device, &image_create_info, NULL, image);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create image");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(painter->device, *image, &mem_requirements);
    Uint32 memory_type_index = _painter_find_memory_type(painter, properties, &mem_requirements);
    VkMemoryAllocateInfo memory_alloc_info;
    memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_alloc_info.pNext = NULL;
    memory_alloc_info.allocationSize = mem_requirements.size;
    memory_alloc_info.memoryTypeIndex = memory_type_index;
    result = vkAllocateMemory(painter->device, &memory_alloc_info, NULL, image_memory);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not allocate image memory");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    vkBindImageMemory(painter->device, *image, *image_memory, 0);
    return SDL_TRUE;
}

SDL_bool _painter_create_image_view(EsPainter* painter, VkFormat format, VkImageAspectFlags aspect_flags, VkImageViewType image_view_type, Uint32 mip_levels, VkImage* image, VkImageView* image_view) {
    VkResult result;
    VkImageViewCreateInfo image_view_create_info;
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.pNext = NULL;
    image_view_create_info.flags = 0;
    image_view_create_info.image = *image;
    image_view_create_info.viewType = image_view_type;
    image_view_create_info.format = format;
    image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.subresourceRange.aspectMask = aspect_flags;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = mip_levels;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    if (image_view_type == VK_IMAGE_VIEW_TYPE_CUBE)
        image_view_create_info.subresourceRange.layerCount = 6;
    else
        image_view_create_info.subresourceRange.layerCount = 1;
    result = vkCreateImageView(painter->device, &image_view_create_info, NULL, image_view);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create image view");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

VkCommandBuffer _painter_begin_single_use_command_buffer(EsPainter* painter) {
    VkResult result;
    VkCommandBufferAllocateInfo command_buffer_allocate_info;
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.pNext = NULL;
    command_buffer_allocate_info.commandPool = painter->command_pool;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = 1;
    VkCommandBuffer command_buffer;
    result = vkAllocateCommandBuffers(painter->device, &command_buffer_allocate_info, &command_buffer);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not allocate command buffers");
        painter_cleanup(painter);
        return NULL;
    }
    VkCommandBufferBeginInfo command_buffer_begin_info;
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.pNext = NULL;
    command_buffer_begin_info.flags = 0;
    command_buffer_begin_info.pInheritanceInfo = NULL;
    result = vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not begin command buffers");
        painter_cleanup(painter);
        return NULL;
    }
    return command_buffer;
}

SDL_bool _painter_end_single_use_command_buffer(EsPainter* painter, VkCommandBuffer* command_buffer) {
    VkResult result;
    result = vkEndCommandBuffer(*command_buffer);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not end command buffers");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = NULL;
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = NULL;
    submit_info.pWaitDstStageMask = NULL;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = command_buffer;
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = NULL;
    result = vkQueueSubmit(painter->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Setup.", "Could not submit to graphics queue: transition");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    vkQueueWaitIdle(painter->presentation_queue);
    vkFreeCommandBuffers(painter->device, painter->command_pool, 1, command_buffer);
    return SDL_TRUE;
}

SDL_bool _painter_generate_mipmaps(EsPainter* painter, VkImage* image, VkFormat image_format, Uint32 width, Uint32 height, Uint32 mip_levels, SDL_bool is_cubemap) {
    SDL_bool sdl_result;
    VkFormatProperties format_properties;
    vkGetPhysicalDeviceFormatProperties(painter->physical_device, image_format, &format_properties);
    if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        warehouse_error_popup("Error in Setup.", "Currently do not support GPUs that don't have linear blitting");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    VkCommandBuffer command_buffer = _painter_begin_single_use_command_buffer(painter);
    if (command_buffer == NULL) return SDL_FALSE;
    VkImageMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = NULL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = *image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.levelCount = 1;
    if (is_cubemap)
        barrier.subresourceRange.layerCount = 6;
    else
        barrier.subresourceRange.layerCount = 1;
    int mip_width = width;
    int mip_height = height;
    for (Uint32 i=1; i<mip_levels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        vkCmdPipelineBarrier(command_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
            0, NULL, 0, NULL, 1, &barrier);
        VkImageBlit blit;
        blit.srcOffsets[0].x = 0;
        blit.srcOffsets[0].y = 0;
        blit.srcOffsets[0].z = 0;
        blit.srcOffsets[1].x = mip_width;
        blit.srcOffsets[1].y = mip_height;
        blit.srcOffsets[1].z = 1;
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        if (is_cubemap)
            blit.srcSubresource.layerCount = 6;
        else
            blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0].x = 0;
        blit.dstOffsets[0].y = 0;
        blit.dstOffsets[0].z = 0;
        if (mip_width > 1)
            blit.dstOffsets[1].x = mip_width / 2;
        else
            blit.dstOffsets[1].x = 1;
        if (mip_height > 1)
            blit.dstOffsets[1].y = mip_height / 2;
        else
            blit.dstOffsets[1].y = 1;
        blit.dstOffsets[1].z = 1;
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        if (is_cubemap)
            blit.dstSubresource.layerCount = 6;
        else
            blit.dstSubresource.layerCount = 1;
        vkCmdBlitImage(command_buffer,
            *image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            *image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit, VK_FILTER_LINEAR);
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(command_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, NULL, 0, NULL, 1, &barrier);
        if (mip_width > 1)
            mip_width /= 2;
        if (mip_height > 1)
            mip_height /= 2;
    }
    barrier.subresourceRange.baseMipLevel = mip_levels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(command_buffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, NULL, 0, NULL, 1, &barrier);
    sdl_result = _painter_end_single_use_command_buffer(painter, &command_buffer);
    if (!sdl_result) return SDL_FALSE;
    return SDL_TRUE;
}

SDL_bool _painter_load_shaders(EsPainter* painter, ShaderData* shader) {
    VkResult result;
    SDL_Log("Loading shaders\n");
    Uint32* vertex_spirv;
    Sint64 vertex_shader_length;
    vertex_shader_length = painter_read_shader_file(shader->vertex_shader, &vertex_spirv);
    if (vertex_shader_length < 0) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not read vertex shader.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    VkShaderModuleCreateInfo vertex_shader_module_create_info;
    vertex_shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertex_shader_module_create_info.pNext = NULL;
    vertex_shader_module_create_info.flags = 0;
    vertex_shader_module_create_info.codeSize = vertex_shader_length;
    vertex_shader_module_create_info.pCode = vertex_spirv;
    result = vkCreateShaderModule(painter->device, &vertex_shader_module_create_info, NULL, &shader->vertex_shader_module);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create vertex shader module.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    Uint32* fragment_spirv;
    Sint64 fragment_shader_length;
    fragment_shader_length = painter_read_shader_file(shader->fragment_shader, &fragment_spirv);
    if (fragment_shader_length < 0) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not read fragment shader.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    VkShaderModuleCreateInfo fragment_shader_module_create_info;
    fragment_shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragment_shader_module_create_info.pNext = NULL;
    fragment_shader_module_create_info.flags = 0;
    fragment_shader_module_create_info.codeSize = fragment_shader_length;
    fragment_shader_module_create_info.pCode = fragment_spirv;
    SDL_Log("Creating shader %s\n", shader->shader_name);
    result = vkCreateShaderModule(painter->device, &fragment_shader_module_create_info, NULL, &shader->fragment_shader_module);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create fragment shader module.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    SDL_free(fragment_spirv);
    SDL_free(vertex_spirv);
    return SDL_TRUE;
}

SDL_bool _painter_create_descriptor_set_layout(EsPainter* painter, ShaderData* shader) {
    VkResult result;
    VkDescriptorSetLayoutBinding ubo_layout_binding;
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = NULL;
    VkDescriptorSetLayoutBinding sampler_layout_binding;
    sampler_layout_binding.binding = 1;
    sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.descriptorCount = 1;
    sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    sampler_layout_binding.pImmutableSamplers = NULL;
    VkDescriptorSetLayoutBinding* bindings = (VkDescriptorSetLayoutBinding*) SDL_malloc(2*sizeof(VkDescriptorSetLayoutBinding));
    bindings[0] = ubo_layout_binding;
    bindings[1] = sampler_layout_binding;
    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info;
    descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_create_info.pNext = NULL;
    descriptor_set_layout_create_info.flags = 0;
    descriptor_set_layout_create_info.bindingCount = 2;
    descriptor_set_layout_create_info.pBindings = bindings;
    result = vkCreateDescriptorSetLayout(painter->device, &descriptor_set_layout_create_info, NULL, &shader->descriptor_set_layout);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create descriptor set layout.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    SDL_free(bindings);
    return SDL_TRUE;
}

SDL_bool _painter_create_pipeline(EsPainter* painter, ShaderData* shader) {
    VkResult result;
    SDL_Log("Describing vertex bindings\n");
    VkVertexInputBindingDescription vertex_input_binding_description;
    vertex_input_binding_description.binding = 0;
    vertex_input_binding_description.stride = sizeof(EsVertex);
    vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    VkVertexInputBindingDescription vertex_binding_descriptions[1];
    vertex_binding_descriptions[0] = vertex_input_binding_description;
    VkVertexInputAttributeDescription vertex_input_attributes[5];
    vertex_input_attributes[0].location = 0;
    vertex_input_attributes[0].binding = 0;
    vertex_input_attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_input_attributes[0].offset = 0;
    vertex_input_attributes[1].location = 1;
    vertex_input_attributes[1].binding = 0;
    vertex_input_attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_input_attributes[1].offset = sizeof(vec3);
    vertex_input_attributes[2].location = 2;
    vertex_input_attributes[2].binding = 0;
    vertex_input_attributes[2].format = VK_FORMAT_R32G32_SFLOAT;
    vertex_input_attributes[2].offset = sizeof(vec3) + sizeof(vec3);
    vertex_input_attributes[3].location = 3;
    vertex_input_attributes[3].binding = 0;
    vertex_input_attributes[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_input_attributes[3].offset = sizeof(vec3) + sizeof(vec3) + sizeof(vec2);
    vertex_input_attributes[4].location = 4;
    vertex_input_attributes[4].binding = 0;
    vertex_input_attributes[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    vertex_input_attributes[4].offset = sizeof(vec3) + sizeof(vec3) + sizeof(vec2) + sizeof(vec3);
    VkPipelineShaderStageCreateInfo vertex_shader_stage_create_info;
    vertex_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_shader_stage_create_info.pNext = NULL;
    vertex_shader_stage_create_info.flags = 0;
    vertex_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_shader_stage_create_info.module = shader->vertex_shader_module;
    vertex_shader_stage_create_info.pName = "main";
    vertex_shader_stage_create_info.pSpecializationInfo = NULL;
    VkPipelineShaderStageCreateInfo fragment_shader_stage_create_info;
    fragment_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_shader_stage_create_info.pNext = NULL;
    fragment_shader_stage_create_info.flags = 0;
    fragment_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_shader_stage_create_info.module = shader->fragment_shader_module;
    fragment_shader_stage_create_info.pName = "main";
    fragment_shader_stage_create_info.pSpecializationInfo = NULL;
    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info;
    vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_create_info.pNext = NULL;
    vertex_input_state_create_info.flags = 0;
    vertex_input_state_create_info.vertexBindingDescriptionCount = 1;
    vertex_input_state_create_info.pVertexBindingDescriptions = vertex_binding_descriptions;
    vertex_input_state_create_info.vertexAttributeDescriptionCount = 5;
    vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_input_attributes;
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info;
    input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state_create_info.pNext = NULL;
    input_assembly_state_create_info.flags = 0;
    input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) painter->swapchain_extent.width;
    viewport.height = (float) painter->swapchain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = painter->swapchain_extent;
    VkPipelineViewportStateCreateInfo viewport_state_create_info;
    viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.pNext = NULL;
    viewport_state_create_info.flags = 0;
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.pViewports = &viewport;
    viewport_state_create_info.scissorCount = 1;
    viewport_state_create_info.pScissors = &scissor;
    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info;
    rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state_create_info.pNext = NULL;
    rasterization_state_create_info.flags = 0;
    rasterization_state_create_info.depthClampEnable = VK_FALSE;
    rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
    // rasterization_state_create_info.cullMode = VK_CULL_MODE_FRONT_BIT;
    rasterization_state_create_info.cullMode = VK_CULL_MODE_NONE; // nocheckin
    rasterization_state_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization_state_create_info.depthBiasEnable = VK_FALSE;
    rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
    rasterization_state_create_info.depthBiasClamp = 0.0f;
    rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;
    rasterization_state_create_info.lineWidth = 1.0f;
    VkPipelineMultisampleStateCreateInfo multisample_state_create_info;
    multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state_create_info.pNext = NULL;
    multisample_state_create_info.flags = 0;
    multisample_state_create_info.rasterizationSamples = painter->msaa_samples;
    multisample_state_create_info.sampleShadingEnable = VK_TRUE;
    multisample_state_create_info.minSampleShading = 0.2f;
    multisample_state_create_info.pSampleMask = NULL;
    multisample_state_create_info.alphaToCoverageEnable = VK_FALSE;
    multisample_state_create_info.alphaToOneEnable = VK_FALSE;
    VkPipelineColorBlendAttachmentState color_blend_attachment_state;
    color_blend_attachment_state.blendEnable = VK_FALSE;
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info;
    color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_create_info.pNext = NULL;
    color_blend_state_create_info.flags = 0;
    color_blend_state_create_info.logicOpEnable = VK_FALSE;
    color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments = &color_blend_attachment_state;
    color_blend_state_create_info.blendConstants[0] = 0.0f;
    color_blend_state_create_info.blendConstants[1] = 0.0f;
    color_blend_state_create_info.blendConstants[2] = 0.0f;
    color_blend_state_create_info.blendConstants[3] = 0.0f;
    VkPipelineLayoutCreateInfo pipeline_layout_create_info;
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.pNext = NULL;
    pipeline_layout_create_info.flags = 0;
    pipeline_layout_create_info.setLayoutCount = 1;
    pipeline_layout_create_info.pSetLayouts = &shader->descriptor_set_layout;
    pipeline_layout_create_info.pushConstantRangeCount = 0;
    pipeline_layout_create_info.pPushConstantRanges = NULL;
    result = vkCreatePipelineLayout(painter->device, &pipeline_layout_create_info, NULL, &shader->pipeline_layout);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create pipeline layout.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }

    SDL_Log("Creating graphics pipeline\n");
    VkPipelineShaderStageCreateInfo shader_stages[2];
    shader_stages[0] = vertex_shader_stage_create_info;
    shader_stages[1] = fragment_shader_stage_create_info;
    VkStencilOpState no_op;
    no_op.failOp = VK_STENCIL_OP_KEEP;
    no_op.passOp = VK_STENCIL_OP_KEEP;
    no_op.depthFailOp = VK_STENCIL_OP_KEEP;
    no_op.compareOp = VK_STENCIL_OP_KEEP ;
    no_op.compareMask = 0;
    no_op.writeMask = 0;
    no_op.reference = 0;
    VkPipelineDepthStencilStateCreateInfo pipeline_depth_stencil_state_create_info;
    pipeline_depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pipeline_depth_stencil_state_create_info.pNext = NULL;
    pipeline_depth_stencil_state_create_info.flags = 0;
    pipeline_depth_stencil_state_create_info.depthTestEnable = VK_TRUE;
    pipeline_depth_stencil_state_create_info.depthWriteEnable = VK_TRUE;
    pipeline_depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
    pipeline_depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
    pipeline_depth_stencil_state_create_info.stencilTestEnable = VK_FALSE;
    pipeline_depth_stencil_state_create_info.front = no_op;
    pipeline_depth_stencil_state_create_info.back = no_op;
    pipeline_depth_stencil_state_create_info.minDepthBounds = 0.0f;
    pipeline_depth_stencil_state_create_info.maxDepthBounds = 1.0f;
    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info;
    graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphics_pipeline_create_info.pNext = NULL;
    graphics_pipeline_create_info.flags = 0;
    graphics_pipeline_create_info.stageCount = 2;
    graphics_pipeline_create_info.pStages = shader_stages;
    graphics_pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
    graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;
    graphics_pipeline_create_info.pTessellationState = NULL;
    graphics_pipeline_create_info.pViewportState = &viewport_state_create_info;
    graphics_pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
    graphics_pipeline_create_info.pMultisampleState = &multisample_state_create_info;
    graphics_pipeline_create_info.pDepthStencilState = &pipeline_depth_stencil_state_create_info;
    graphics_pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
    graphics_pipeline_create_info.pDynamicState = NULL;
    graphics_pipeline_create_info.layout = shader->pipeline_layout;
    graphics_pipeline_create_info.renderPass = painter->render_pass;
    graphics_pipeline_create_info.subpass = 0;
    graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    graphics_pipeline_create_info.basePipelineIndex = -1;
    result = vkCreateGraphicsPipelines(painter->device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, NULL, &shader->pipeline);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create graphics pipeline");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

SDL_bool _painter_init_shader_data(EsPainter* painter, ShaderData* shader, SDL_bool is_skybox) {
    SDL_bool sdl_result;
    sdl_result = _painter_load_shaders(painter, shader);
    if (!sdl_result) return SDL_FALSE;
    sdl_result = _painter_create_descriptor_set_layout(painter, shader);
    if (!sdl_result) return SDL_FALSE;
    sdl_result = _painter_create_pipeline(painter, shader);
    if (!sdl_result) return SDL_FALSE;
    if (is_skybox) 
         sdl_result = _painter_load_cubemap_and_sampler(painter, SKYBOX_MODEL_TEXTURE_PATH0, SKYBOX_MODEL_TEXTURE_PATH1, SKYBOX_MODEL_TEXTURE_PATH2, SKYBOX_MODEL_TEXTURE_PATH3, SKYBOX_MODEL_TEXTURE_PATH4, SKYBOX_MODEL_TEXTURE_PATH5, shader);
    else
        sdl_result = _painter_load_image_and_sampler(painter, shader->texture_filepath, shader);
    if (!sdl_result) return _painter_custom_error("Texture Load Failed", shader->shader_name);

    VkMemoryPropertyFlags vertex_staging_property_flags =  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkMemoryPropertyFlags vertex_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkMemoryPropertyFlags index_staging_property_flags =  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkMemoryPropertyFlags index_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    shader->vertex_staging_buffer_size = shader->num_vertices * sizeof(EsVertex);
    // TODO (21 Oct 2020 sam): Use a single vkAllocateMemory for both buffers, and manage memory using
    // the offsets and things.
    sdl_result = _painter_create_buffer(painter, shader->vertex_staging_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vertex_staging_property_flags, &shader->vertex_staging_buffer, &shader->vertex_staging_buffer_memory);
    if (!sdl_result) return SDL_FALSE;
    shader->vertex_buffer_size = shader->num_vertices * sizeof(EsVertex);
    sdl_result = _painter_create_buffer(painter, shader->vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertex_property_flags, &shader->vertex_buffer, &shader->vertex_buffer_memory);
    if (!sdl_result) return SDL_FALSE;

    shader->index_staging_buffer_size = shader->num_indices * sizeof(Uint32);
    // TODO (21 Oct 2020 sam): Use a single vkAllocateMemory for both buffers, and manage memory using
    // the offsets and things.
    SDL_Log("num vertices = %i num indices = %i\n", shader->num_vertices, shader->num_indices);
    sdl_result = _painter_create_buffer(painter, shader->index_staging_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, index_staging_property_flags, &shader->index_staging_buffer, &shader->index_staging_buffer_memory);
    if (!sdl_result) return SDL_FALSE;
    shader->index_buffer_size = shader->num_indices * sizeof(Uint32);
    sdl_result = _painter_create_buffer(painter, shader->index_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, index_property_flags, &shader->index_buffer, &shader->index_buffer_memory);
    if (!sdl_result) return SDL_FALSE;

    sdl_result = _painter_load_buffer_via_staging(painter, shader->vertices, &shader->vertex_staging_buffer_memory, &shader->vertex_staging_buffer, &shader->vertex_buffer, shader->vertex_staging_buffer_size);
    if (!sdl_result) _painter_cleanup_error(painter, "Could not copy vertices to buffer.", shader->shader_name);
    sdl_result = _painter_load_buffer_via_staging(painter, shader->indices, &shader->index_staging_buffer_memory, &shader->index_staging_buffer, &shader->index_buffer, shader->index_staging_buffer_size);
    if (!sdl_result) _painter_cleanup_error(painter, "Could not copy indices to buffer.", shader->shader_name);
    return SDL_TRUE;
}
