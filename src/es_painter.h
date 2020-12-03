/* 
 * es_painter is the rendering system for easel.
 */

#ifndef ES_PAINTER_DEFINED
#define ES_PAINTER_DEFINED

#include "SDL.h"
#include <vulkan/vulkan.h>
#include "es_warehouse.h"
#include "es_geometrygen.h"
#include "es_trees.h"
#include "es_world.h"

typedef struct {
    vec3 pos;
    vec3 color;
    vec2 tex;
    vec3 normal;
} EsVertex;

typedef struct {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 camera_position;
    float time;
} UniformBufferObject;

typedef struct {
    const char* shader_name;
    const char* vertex_shader;
    const char* fragment_shader;
    const char* texture_filepath;
    EsVertex* vertices;
    Uint32* indices;
    Uint32 num_vertices;
    Uint32 num_indices;
    Uint32 mip_levels;
    Uint32 vertex_buffer_size;
    Uint32 vertex_staging_buffer_size;
    Uint32 index_buffer_size;
    Uint32 index_staging_buffer_size;
    VkBuffer vertex_staging_buffer;
    VkDeviceMemory vertex_staging_buffer_memory;
    VkBuffer index_staging_buffer;
    VkDeviceMemory index_staging_buffer_memory;
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;
    VkBuffer index_buffer;
    VkDeviceMemory index_buffer_memory;
    VkImage texture_image;
    VkDeviceMemory texture_image_memory;
    VkImageView texture_image_view;
    VkSampler texture_sampler;
    VkDescriptorSetLayout descriptor_set_layout;
    VkShaderModule vertex_shader_module;
    VkShaderModule fragment_shader_module;
    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet* descriptor_sets;
} ShaderData;

typedef struct {
    SDL_Window* window;
    VkInstance instance;
    VkSurfaceKHR surface;
    VkDevice device;
    VkPhysicalDevice physical_device;
    VkSwapchainKHR swapchain;
    Uint32 swapchain_image_count;
    Uint32 frame_index;
    VkImageView* swapchain_image_views;
    VkExtent2D swapchain_extent;
    VkRenderPass render_pass;
    VkFramebuffer* swapchain_framebuffers;
    VkCommandPool command_pool;
    VkSemaphore* image_available_semaphores;
    VkSemaphore* render_finished_semaphores;
    VkFence* in_flight_fences;
    VkFence* images_in_flight;
    VkQueue presentation_queue;
    VkFormat swapchain_image_format;
    VkCommandBuffer* command_buffers;
    VkQueue graphics_queue;
    VkSampleCountFlagBits msaa_samples;
    SDL_bool buffer_resized;
    Uint32 uniform_buffer_size;
    UniformBufferObject uniform_buffer_object;
    VkBuffer* uniform_buffers;
    VkDeviceMemory* uniform_buffers_memory;
    VkImageView color_image_view;
    VkImage color_image;
    VkDeviceMemory color_image_memory;
    VkImage depth_image;
    VkDeviceMemory depth_image_memory;
    VkImageView depth_image_view;
    vec3 camera_position;
    float camera_fov;
    Uint32 start_time;
    ShaderData* skybox_shader;
    Uint32 num_shaders;
    ShaderData* shaders;
    EsWorld* world;
} EsPainter;

extern SDL_bool painter_initialise(EsPainter* painter);
extern SDL_bool painter_paint_frame(EsPainter* painter);
extern void painter_cleanup(EsPainter* painter);
extern Sint64 painter_read_shader_file(const char* filename, Uint32** buffer);

#endif
