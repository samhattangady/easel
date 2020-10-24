#include "es_painter.h"
#include "SDL_vulkan.h"

#define HARDCODED_NUM_VERTICES 4
#define HARDCODED_NUM_INDICES 6

void _painter_cleanup_swapchain(EsPainter* painter);
SDL_bool _painter_create_swapchain(EsPainter* painter);
SDL_bool _painter_recreate_swapchain(EsPainter* painter);
SDL_bool _painter_create_buffer(EsPainter* painter, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory);

SDL_bool painter_initialise(EsPainter* painter) {
    int init_success = SDL_Init(SDL_INIT_VIDEO);
    if (init_success !=0) {
        warehouse_error_popup("Error in SDL Initialisation.", SDL_GetError());
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    SDL_Window* window;
    window = SDL_CreateWindow("Easel", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              1024, 768, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    if (window == NULL) {
        warehouse_error_popup("Error in SDL Window Creation.", SDL_GetError());
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    painter->window = window;
    painter->start_time = SDL_GetTicks();

    VkResult result;
    VkApplicationInfo app_info;
    SDL_bool sdl_result;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = NULL;
    app_info.pApplicationName = "Easel";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "chapliboy engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    painter->vertices = (TutorialVertex*) SDL_malloc(HARDCODED_NUM_VERTICES * sizeof(TutorialVertex));
    vec2 pos1 = {-0.5f, -0.5f};
    vec3 color1 = {1.0f, 1.0f, 1.0f};
    vec2 pos2 = {0.5f, -0.5f};
    vec3 color2 = {0.0f, 1.0f, 0.0f};
    vec2 pos3 = {0.5f, 0.5f};
    vec3 color3 = {1.0f, 1.0f, 1.0f};
    vec2 pos4 = {-0.5f, 0.5f};
    vec3 color4 = {0.0f, 0.0f, 1.0f};
    painter->vertices[0].pos = pos1;
    painter->vertices[0].color = color1;
    painter->vertices[1].pos = pos2;
    painter->vertices[1].color = color2;
    painter->vertices[2].pos = pos3;
    painter->vertices[2].color = color3;
    painter->vertices[3].pos = pos4;
    painter->vertices[3].color = color4;
    painter->indices = (Uint32*) SDL_malloc(HARDCODED_NUM_INDICES * sizeof(Uint32));
    painter->indices[0] = 0;
    painter->indices[1] = 1;
    painter->indices[2] = 2;
    painter->indices[3] = 2;
    painter->indices[4] = 3;
    painter->indices[5] = 0;

    painter->uniform_buffer_object.model.a.x = 1.0;
    painter->uniform_buffer_object.model.a.y = 0.0;
    painter->uniform_buffer_object.model.a.z = 0.0;
    painter->uniform_buffer_object.model.a.w = 0.0;
    painter->uniform_buffer_object.model.b.x = 0.0;
    painter->uniform_buffer_object.model.b.y = 1.0;
    painter->uniform_buffer_object.model.b.z = 0.0;
    painter->uniform_buffer_object.model.b.w = 0.0;
    painter->uniform_buffer_object.model.c.x = 0.0;
    painter->uniform_buffer_object.model.c.y = 0.0;
    painter->uniform_buffer_object.model.c.z = 1.0;
    painter->uniform_buffer_object.model.c.w = 0.0;
    painter->uniform_buffer_object.model.d.x = 0.0;
    painter->uniform_buffer_object.model.d.y = 0.0;
    painter->uniform_buffer_object.model.d.z = 0.0;
    painter->uniform_buffer_object.model.d.w = 1.0;

    painter->uniform_buffer_object.view.a.x = -0.707107;
    painter->uniform_buffer_object.view.a.y = -0.408248;
    painter->uniform_buffer_object.view.a.z = 0.577350;
    painter->uniform_buffer_object.view.a.w = 0.000000;
    painter->uniform_buffer_object.view.b.x = 0.707107;
    painter->uniform_buffer_object.view.b.y = -0.408248;
    painter->uniform_buffer_object.view.b.z = 0.577350;
    painter->uniform_buffer_object.view.b.w = 0.000000;
    painter->uniform_buffer_object.view.c.x = 0.000000;
    painter->uniform_buffer_object.view.c.y = 0.816497;
    painter->uniform_buffer_object.view.c.z = 0.577350;
    painter->uniform_buffer_object.view.c.w = 0.000000;
    painter->uniform_buffer_object.view.d.x = -0.000000;
    painter->uniform_buffer_object.view.d.y = -0.000000;
    painter->uniform_buffer_object.view.d.z = -3.464102;
    painter->uniform_buffer_object.view.d.w = 1.000000;

    painter->uniform_buffer_object.proj.a.x = 3.218951;
    painter->uniform_buffer_object.proj.a.y = 0.000000;
    painter->uniform_buffer_object.proj.a.z = 0.000000;
    painter->uniform_buffer_object.proj.a.w = 0.000000;
    painter->uniform_buffer_object.proj.b.x = 0.000000;
    painter->uniform_buffer_object.proj.b.y = -2.414213;
    painter->uniform_buffer_object.proj.b.z = 0.000000;
    painter->uniform_buffer_object.proj.b.w = 0.000000;
    painter->uniform_buffer_object.proj.c.x = 0.000000;
    painter->uniform_buffer_object.proj.c.y = 0.000000;
    painter->uniform_buffer_object.proj.c.z = -1.020202;
    painter->uniform_buffer_object.proj.c.w = -1.000000;
    painter->uniform_buffer_object.proj.d.x = 0.000000;
    painter->uniform_buffer_object.proj.d.y = 0.000000;
    painter->uniform_buffer_object.proj.d.z = -0.202020;
    painter->uniform_buffer_object.proj.d.w = 0.000000;

#if DEBUG_BUILD==SDL_TRUE
    const Uint32 validation_layers_count = 1;
    const char** validation_layers = (char**) SDL_malloc(validation_layers_count * sizeof(char*));
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
    required_extensions = (char**) SDL_malloc(required_extensions_count * sizeof(char*));
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

    sdl_result = _painter_create_swapchain(painter);
    if (!sdl_result) {
        // The error would be called from the _painter_create_swapchain method.
        return SDL_FALSE;
    }

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
    painter->frame_index = 0;
    painter->buffer_resized = SDL_FALSE;

#if DEBUG_BUILD==SDL_TRUE
    SDL_free((char*)validation_layers);
#endif
    SDL_free(devices);
    SDL_free((char*)required_extensions);
    SDL_free(available_extensions);

    return SDL_TRUE;
}

SDL_bool _painter_create_swapchain(EsPainter* painter) {
    VkResult result;
    SDL_bool sdl_result;

    const Uint32 required_device_extensions_count = 1;
    const char** required_device_extensions = (char**) SDL_malloc(required_device_extensions_count * sizeof(char*));
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
    // This method does not return a VkResult
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
    SDL_Log("Graphics Queue Family is %i\n", graphics_queue_family);
    SDL_Log("Presentation Queue Family is %i\n", presentation_queue_family);
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
    const char** validation_layers = (char**) SDL_malloc(validation_layers_count * sizeof(char*));
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
    VkPhysicalDeviceFeatures device_features = {VK_FALSE};
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
    VkFormat swapchain_image_format = swapchain_create_info.imageFormat;
    VkExtent2D swapchain_extent = swapchain_create_info.imageExtent;
    painter->swapchain_image_views = (VkImageView*) SDL_malloc(painter->swapchain_image_count * sizeof(VkImageView));
    for (Uint32 i=0; i<painter->swapchain_image_count; i++) {
        VkImageViewCreateInfo imageview_create_info;
        imageview_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageview_create_info.pNext = NULL;
        imageview_create_info.flags = 0;
        imageview_create_info.image = swapchain_images[i];
        imageview_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageview_create_info.format = swapchain_image_format;
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

    VkAttachmentDescription color_attachment;
    color_attachment.flags = 0;
    color_attachment.format = swapchain_image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference color_attachment_reference;
    color_attachment_reference.attachment = 0;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass;
    subpass.flags = 0;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_reference;
    subpass.pResolveAttachments = 0;
    subpass.pDepthStencilAttachment = NULL;
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
    VkRenderPassCreateInfo render_pass_create_info;
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.pNext = NULL;
    render_pass_create_info.flags = 0;
    render_pass_create_info.attachmentCount = 1;
    render_pass_create_info.pAttachments = &color_attachment;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass;
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies = &subpass_dependency;
    result = vkCreateRenderPass(painter->device, &render_pass_create_info, NULL, &painter->render_pass);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create render pass.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }

    SDL_Log("Loading shaders\n");
    Uint32* vertex_spirv;
    Sint64 vertex_shader_length;
    vertex_shader_length = painter_read_shader_file("data/spirv/vertex.spv", &vertex_spirv);
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
    result = vkCreateShaderModule(painter->device, &vertex_shader_module_create_info, NULL, &painter->vertex_shader_module);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create vertex shader module.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    Uint32* fragment_spirv;
    Sint64 fragment_shader_length;
    fragment_shader_length = painter_read_shader_file("data/spirv/fragment.spv", &fragment_spirv);
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
    result = vkCreateShaderModule(painter->device, &fragment_shader_module_create_info, NULL, &painter->fragment_shader_module);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create fragment shader module.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }

    VkDescriptorSetLayoutBinding ubo_layout_binding;
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = NULL;
    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info;
    descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_create_info.pNext = NULL;
    descriptor_set_layout_create_info.flags = 0;
    descriptor_set_layout_create_info.bindingCount = 1;
    descriptor_set_layout_create_info.pBindings = &ubo_layout_binding;
    result = vkCreateDescriptorSetLayout(painter->device, &descriptor_set_layout_create_info, NULL, &painter->descriptor_set_layout);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create descriptor set layout.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }

    SDL_Log("Describing vertex bindings\n");
    VkVertexInputBindingDescription vertex_input_binding_description;
    vertex_input_binding_description.binding = 0;
    vertex_input_binding_description.stride = sizeof(TutorialVertex);
    vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    VkVertexInputAttributeDescription vertex_input_attributes[2];
    vertex_input_attributes[0].location = 0;
    vertex_input_attributes[0].binding = 0;
    vertex_input_attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
    vertex_input_attributes[0].offset = 0;
    vertex_input_attributes[1].location = 1;
    vertex_input_attributes[1].binding = 0;
    vertex_input_attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_input_attributes[1].offset = sizeof(vec2);
    VkPipelineShaderStageCreateInfo vertex_shader_stage_create_info;
    vertex_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_shader_stage_create_info.pNext = NULL;
    vertex_shader_stage_create_info.flags = 0;
    vertex_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_shader_stage_create_info.module = painter->vertex_shader_module;
    vertex_shader_stage_create_info.pName = "main";
    vertex_shader_stage_create_info.pSpecializationInfo = NULL;
    VkPipelineShaderStageCreateInfo fragment_shader_stage_create_info;
    fragment_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_shader_stage_create_info.pNext = NULL;
    fragment_shader_stage_create_info.flags = 0;
    fragment_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_shader_stage_create_info.module = painter->fragment_shader_module;
    fragment_shader_stage_create_info.pName = "main";
    fragment_shader_stage_create_info.pSpecializationInfo = NULL;
    VkPipelineShaderStageCreateInfo shader_stages[2];
    shader_stages[0] = vertex_shader_stage_create_info;
    shader_stages[1] = fragment_shader_stage_create_info;
    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info;
    vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_create_info.pNext = NULL;
    vertex_input_state_create_info.flags = 0;
    vertex_input_state_create_info.vertexBindingDescriptionCount = 1;
    vertex_input_state_create_info.pVertexBindingDescriptions = &vertex_input_binding_description;
    vertex_input_state_create_info.vertexAttributeDescriptionCount = 2;
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
    viewport.width = (float) swapchain_extent.width;
    viewport.height = (float) swapchain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = swapchain_extent;
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
    multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_state_create_info.sampleShadingEnable = VK_FALSE;
    multisample_state_create_info.minSampleShading = 1.0f;
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
    pipeline_layout_create_info.pSetLayouts = &painter->descriptor_set_layout;
    pipeline_layout_create_info.pushConstantRangeCount = 0;
    pipeline_layout_create_info.pPushConstantRanges = NULL;
    result = vkCreatePipelineLayout(painter->device, &pipeline_layout_create_info, NULL, &painter->pipeline_layout);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create pipeline layout.");
        painter_cleanup(painter);
        return SDL_FALSE;
    }

    SDL_Log("Creating graphics pipeline\n");
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
    graphics_pipeline_create_info.pDepthStencilState = NULL;
    graphics_pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
    graphics_pipeline_create_info.pDynamicState = NULL;
    graphics_pipeline_create_info.layout = painter->pipeline_layout;
    graphics_pipeline_create_info.renderPass = painter->render_pass;
    graphics_pipeline_create_info.subpass = 0;
    graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    graphics_pipeline_create_info.basePipelineIndex = -1;
    result = vkCreateGraphicsPipelines(painter->device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, NULL, &painter->graphics_pipeline);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create graphics pipeline");
        painter_cleanup(painter);
        return SDL_FALSE;
    }

    painter->swapchain_framebuffers = (VkFramebuffer*) SDL_malloc(painter->swapchain_image_count * sizeof(VkFramebuffer));
    for (Uint32 i=0; i<painter->swapchain_image_count; i++) {
        VkImageView attachments[1];
        attachments[0] = painter->swapchain_image_views[i];
        VkFramebufferCreateInfo framebuffer_create_info;
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.pNext = NULL;
        framebuffer_create_info.flags = 0;
        framebuffer_create_info.renderPass = painter->render_pass;
        framebuffer_create_info.attachmentCount = 1;
        framebuffer_create_info.pAttachments = attachments;
        framebuffer_create_info.width = swapchain_extent.width;
        framebuffer_create_info.height = swapchain_extent.height;
        framebuffer_create_info.layers = 1;
        result = vkCreateFramebuffer(painter->device, &framebuffer_create_info, NULL, &painter->swapchain_framebuffers[i]);
        if (result != VK_SUCCESS) {
            warehouse_error_popup("Error in Vulkan Setup.", "Could not create framebuffer");
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

    painter->vertex_staging_buffer_size = HARDCODED_NUM_VERTICES * sizeof(TutorialVertex);
    VkMemoryPropertyFlags vertex_staging_property_flags =  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    // TODO (21 Oct 2020 sam): Use a single vkAllocateMemory for both buffers, and manage memory using
    // the offsets and things.
    sdl_result = _painter_create_buffer(painter, painter->vertex_staging_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vertex_staging_property_flags, &painter->vertex_staging_buffer, &painter->vertex_staging_buffer_memory);
    if (!sdl_result) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create staging buffer");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    void* staging_data;
    result = vkMapMemory(painter->device, painter->vertex_staging_buffer_memory, 0, painter->vertex_staging_buffer_size, 0, &staging_data);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not map staging buffer memory");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    SDL_memcpy(staging_data, painter->vertices, (size_t) painter->vertex_staging_buffer_size);
    vkUnmapMemory(painter->device, painter->vertex_staging_buffer_memory);
    painter->vertex_buffer_size = HARDCODED_NUM_VERTICES * sizeof(TutorialVertex);
    VkMemoryPropertyFlags vertex_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    sdl_result = _painter_create_buffer(painter, painter->vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertex_property_flags, &painter->vertex_buffer, &painter->vertex_buffer_memory);
    if (!sdl_result) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create vertex buffer");
        painter_cleanup(painter);
        return SDL_FALSE;
    }

    painter->index_staging_buffer_size = HARDCODED_NUM_INDICES * sizeof(Uint32);
    VkMemoryPropertyFlags index_staging_property_flags =  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    // TODO (21 Oct 2020 sam): Use a single vkAllocateMemory for both buffers, and manage memory using
    // the offsets and things.
    sdl_result = _painter_create_buffer(painter, painter->index_staging_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, index_staging_property_flags, &painter->index_staging_buffer, &painter->index_staging_buffer_memory);
    if (!sdl_result) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create staging buffer");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    void* index_staging_data;
    result = vkMapMemory(painter->device, painter->index_staging_buffer_memory, 0, painter->index_staging_buffer_size, 0, &index_staging_data);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not map staging buffer memory");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    SDL_memcpy(index_staging_data, painter->indices, (size_t) painter->index_staging_buffer_size);
    vkUnmapMemory(painter->device, painter->index_staging_buffer_memory);
    painter->index_buffer_size = HARDCODED_NUM_INDICES * sizeof(Uint32);
    VkMemoryPropertyFlags index_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    sdl_result = _painter_create_buffer(painter, painter->index_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, index_property_flags, &painter->index_buffer, &painter->index_buffer_memory);
    if (!sdl_result) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create index buffer");
        painter_cleanup(painter);
        return SDL_FALSE;
    }

    painter->uniform_buffer_size = sizeof(UniformBufferObject);
    painter->uniform_buffers = (VkBuffer*) SDL_malloc(painter->swapchain_image_count * sizeof(VkBuffer));
    painter->uniform_buffers_memory = (VkDeviceMemory*) SDL_malloc(painter->swapchain_image_count * sizeof(VkDeviceMemory));
    for (Uint32 i=0; i<painter->swapchain_image_count; i++) {
        sdl_result = _painter_create_buffer(painter, painter->uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &painter->uniform_buffers[i], &painter->uniform_buffers_memory[i]);
        if (!sdl_result) {
            warehouse_error_popup("Error in Vulkan Setup.", "Could not create uniform buffer");
            painter_cleanup(painter);
            return SDL_FALSE;
        }
    }

    VkDescriptorPoolSize descriptor_pool_size;
    descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_pool_size.descriptorCount = painter->swapchain_image_count;
    VkDescriptorPoolCreateInfo descriptor_pool_info;
    descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_info.pNext = NULL;
    descriptor_pool_info.flags = 0;
    descriptor_pool_info.maxSets = painter->swapchain_image_count;
    descriptor_pool_info.poolSizeCount = 1;
    descriptor_pool_info.pPoolSizes = &descriptor_pool_size;
    result = vkCreateDescriptorPool(painter->device, &descriptor_pool_info, NULL, &painter->descriptor_pool);
    if (!sdl_result) {
        warehouse_error_popup("Error in Vulkan Setup.", "Could not create descriptor pool");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    VkDescriptorSetLayout* layouts = (VkDescriptorSetLayout*) SDL_malloc(painter->swapchain_image_count * sizeof(VkDescriptorSetLayout));
    for (Uint32 i=0; i<painter->swapchain_image_count; i++)
        layouts[i] = painter->descriptor_set_layout;
    painter->descriptor_sets = (VkDescriptorSet*) SDL_malloc(painter->swapchain_image_count * sizeof(VkDescriptorSet));
    VkDescriptorSetAllocateInfo descriptor_set_alloc_info;
    descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_set_alloc_info.pNext = NULL;
    descriptor_set_alloc_info.descriptorPool = painter->descriptor_pool;
    descriptor_set_alloc_info.descriptorSetCount = painter->swapchain_image_count;
    descriptor_set_alloc_info.pSetLayouts = layouts;
    result = vkAllocateDescriptorSets(painter->device, &descriptor_set_alloc_info, painter->descriptor_sets);
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
        VkWriteDescriptorSet descriptor_write;
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.pNext = NULL;
        descriptor_write.dstSet = painter->descriptor_sets[i];
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = &buffer_info;
        descriptor_write.pImageInfo = NULL;
        descriptor_write.pTexelBufferView = NULL;
        vkUpdateDescriptorSets(painter->device, 1, &descriptor_write, 0, NULL);
    }

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
        VkBufferCopy vertex_buffer_copy_info;
        vertex_buffer_copy_info.srcOffset = 0;
        vertex_buffer_copy_info.dstOffset = 0;
        vertex_buffer_copy_info.size = painter->vertex_staging_buffer_size;
        VkBufferCopy index_buffer_copy_info;
        index_buffer_copy_info.srcOffset = 0;
        index_buffer_copy_info.dstOffset = 0;
        index_buffer_copy_info.size = painter->index_staging_buffer_size;
        VkRenderPassBeginInfo render_pass_begin_info;
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.pNext = NULL;
        render_pass_begin_info.renderPass = painter->render_pass;
        render_pass_begin_info.framebuffer = painter->swapchain_framebuffers[i];
        render_pass_begin_info.renderArea.offset.x = 0;
        render_pass_begin_info.renderArea.offset.y = 0;
        render_pass_begin_info.renderArea.extent = swapchain_extent;
        VkClearValue clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
        render_pass_begin_info.clearValueCount = 1;
        render_pass_begin_info.pClearValues = &clear_color;
        VkBuffer vertex_buffers[1];
        vertex_buffers[0] = painter->vertex_buffer;
        VkDeviceSize offsets[1];
        offsets[0] = 0;
        vkCmdCopyBuffer(painter->command_buffers[i], painter->vertex_staging_buffer, painter->vertex_buffer, 1, &vertex_buffer_copy_info);
        vkCmdCopyBuffer(painter->command_buffers[i], painter->index_staging_buffer, painter->index_buffer, 1, &index_buffer_copy_info);
        vkCmdBeginRenderPass(painter->command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(painter->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, painter->graphics_pipeline);
        vkCmdBindVertexBuffers(painter->command_buffers[i], 0, 1, vertex_buffers, offsets);
        vkCmdBindIndexBuffer(painter->command_buffers[i], painter->index_buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(painter->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, painter->pipeline_layout, 0, 1, &painter->descriptor_sets[i], 0, NULL);
        vkCmdDrawIndexed(painter->command_buffers[i], HARDCODED_NUM_INDICES, 1, 0, 0, 0);
        vkCmdEndRenderPass(painter->command_buffers[i]);
        result = vkEndCommandBuffer(painter->command_buffers[i]);
        if (result != VK_SUCCESS) {
            warehouse_error_popup("Error in Vulkan Setup.", "Could not end command buffers");
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
    SDL_free(fragment_spirv);
    SDL_free(vertex_spirv);
    SDL_free(device_extensions);
    return SDL_TRUE;
}

SDL_bool painter_paint_frame(EsPainter* painter) {
    // TODO (20 Oct 2020 sam): We need to handle the case of minimized frames.
    Uint32 image_index;
    VkResult result;
    SDL_bool sdl_result;

    // painter->vertices[0].color.x += 0.00001;
    // painter->vertices[1].pos.x += 0.00001;
    void* vertex_staging_data;
    result = vkMapMemory(painter->device, painter->vertex_staging_buffer_memory, 0, painter->vertex_staging_buffer_size, 0, &vertex_staging_data);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Rendering.", "Could not map staging buffer memory");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    SDL_memcpy(vertex_staging_data, painter->vertices, (size_t) painter->vertex_staging_buffer_size);
    vkUnmapMemory(painter->device, painter->vertex_staging_buffer_memory);

    void* index_staging_data;
    result = vkMapMemory(painter->device, painter->index_staging_buffer_memory, 0, painter->index_staging_buffer_size, 0, &index_staging_data);
    if (result != VK_SUCCESS) {
        warehouse_error_popup("Error in Rendering.", "Could not map staging buffer memory");
        painter_cleanup(painter);
        return SDL_FALSE;
    }
    SDL_memcpy(index_staging_data, painter->indices, (size_t) painter->index_staging_buffer_size);
    vkUnmapMemory(painter->device, painter->index_staging_buffer_memory);

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

void _painter_cleanup_swapchain(EsPainter* painter) {
    vkQueueWaitIdle(painter->presentation_queue);
    vkDeviceWaitIdle(painter->device);
    if (painter->swapchain_framebuffers) {
        for (Uint32 i=0; i<painter->swapchain_image_count; i++)
            vkDestroyFramebuffer(painter->device, painter->swapchain_framebuffers[i], NULL);
    }
    if (painter->command_buffers)
        vkFreeCommandBuffers(painter->device, painter->command_pool, painter->swapchain_image_count, painter->command_buffers);
    if (painter->graphics_pipeline)
        vkDestroyPipeline(painter->device, painter->graphics_pipeline, NULL);
    if (painter->descriptor_set_layout)
        vkDestroyDescriptorSetLayout(painter->device, painter->descriptor_set_layout, NULL);
    if (painter->pipeline_layout)
        vkDestroyPipelineLayout(painter->device, painter->pipeline_layout, NULL);
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

void painter_cleanup(EsPainter* painter) {
    _painter_cleanup_swapchain(painter);
    if (painter->uniform_buffers)
        for (Uint32 i=0; i<painter->swapchain_image_count; i++)
            vkDestroyBuffer(painter->device, painter->uniform_buffers[i], NULL);
    if (painter->uniform_buffers_memory)
        for (Uint32 i=0; i<painter->swapchain_image_count; i++)
            vkFreeMemory(painter->device, painter->uniform_buffers_memory[i], NULL);
    if (painter->descriptor_pool)
        vkDestroyDescriptorPool(painter->device, painter->descriptor_pool, NULL);
    if (painter->index_buffer)
        vkDestroyBuffer(painter->device, painter->index_buffer, NULL);
    if (painter->index_buffer_memory)
        vkFreeMemory(painter->device, painter->index_buffer_memory, NULL);
    if (painter->index_staging_buffer)
        vkDestroyBuffer(painter->device, painter->index_staging_buffer, NULL);
    if (painter->index_staging_buffer_memory)
        vkFreeMemory(painter->device, painter->index_staging_buffer_memory, NULL);
    if (painter->vertex_buffer)
        vkDestroyBuffer(painter->device, painter->vertex_buffer, NULL);
    if (painter->vertex_buffer_memory)
        vkFreeMemory(painter->device, painter->vertex_buffer_memory, NULL);
    if (painter->vertex_staging_buffer)
        vkDestroyBuffer(painter->device, painter->vertex_staging_buffer, NULL);
    if (painter->vertex_staging_buffer_memory)
        vkFreeMemory(painter->device, painter->vertex_staging_buffer_memory, NULL);
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
    if (painter->vertex_shader_module)
        vkDestroyShaderModule(painter->device, painter->vertex_shader_module, NULL);
    if (painter->fragment_shader_module)
        vkDestroyShaderModule(painter->device, painter->fragment_shader_module, NULL);
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
    Uint32 memory_type_index = UINT32_MAX;
    VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
    vkGetPhysicalDeviceMemoryProperties(painter->physical_device, &physical_device_memory_properties);
    for (Uint32 i=0; i<physical_device_memory_properties.memoryTypeCount; i++) {
        SDL_bool memory_is_suitable = memory_requirements.memoryTypeBits & (1<<i);
        SDL_bool memory_has_properties = (physical_device_memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags; 
        if (memory_is_suitable && memory_has_properties) {
            memory_type_index = i;
            break;
        }
    }
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

