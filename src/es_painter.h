/* 
 * es_painter is the rendering system for easel.
 */

#include "SDL.h"
#include <vulkan/vulkan.h>
#include "es_warehouse.h"

typedef struct {
    mat4 model;
    mat4 view;
    mat4 proj;
} UniformBufferObject;

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
    VkShaderModule vertex_shader_module;
    VkShaderModule fragment_shader_module;
    VkDescriptorSetLayout descriptor_set_layout;
    VkPipelineLayout pipeline_layout;
    VkRenderPass render_pass;
    VkPipeline graphics_pipeline;
    VkFramebuffer* swapchain_framebuffers;
    VkCommandPool command_pool;
    VkSemaphore* image_available_semaphores;
    VkSemaphore* render_finished_semaphores;
    VkFence* in_flight_fences;
    VkFence* images_in_flight;
    VkQueue presentation_queue;
    VkCommandBuffer* command_buffers;
    VkQueue graphics_queue;
    SDL_bool buffer_resized;
    Uint32 vertex_buffer_size;
    Uint32 vertex_staging_buffer_size;
    Uint32 index_buffer_size;
    Uint32 index_staging_buffer_size;
    Uint32 uniform_buffer_size;
    VkBuffer vertex_staging_buffer;
    VkDeviceMemory vertex_staging_buffer_memory;
    VkBuffer index_staging_buffer;
    VkDeviceMemory index_staging_buffer_memory;
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;
    VkBuffer index_buffer;
    VkDeviceMemory index_buffer_memory;
    VkBuffer* uniform_buffers;
    VkDeviceMemory* uniform_buffers_memory;
    VkImage texture_image;
    VkDeviceMemory texture_image_memory;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet* descriptor_sets;
    VkImageView texture_image_view;
    VkSampler texture_sampler;
    VkImage depth_image;
    VkDeviceMemory depth_image_memory;
    VkImageView depth_image_view;
    TutorialVertex* vertices;
    Uint32* indices;
    UniformBufferObject uniform_buffer_object;
    Uint32 start_time;
} EsPainter;

extern SDL_bool painter_initialise(EsPainter* painter);
extern SDL_bool painter_paint_frame(EsPainter* painter);
extern void painter_cleanup(EsPainter* painter);
extern Sint64 painter_read_shader_file(const char* filename, Uint32** buffer);
