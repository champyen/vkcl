#ifndef _VKCL_H_
#define _VKCL_H_


#include <vulkan/vulkan.h>

typedef struct{
    VkInstance                          instance;
    VkPhysicalDevice                    phy_dev;
    VkDevice                            dev;
    VkPhysicalDeviceMemoryProperties    phy_mem_prop;
    VkQueue                             queue;
    uint32_t                            qfam_idx;
} vkcl_context;

typedef struct{
    VkMemoryRequirements                mem_req;
    VkMemoryAllocateInfo                info;
    VkDeviceMemory                      memory;
    VkDeviceSize                        size;
    uint32_t                            type;
    void                                *ptr;
    vkcl_context                        *ctx;
} vkcl_memory;


#define MAX_DESCRIPTOR                  16
typedef struct{
    uint32_t                            set_id;
    VkDescriptorSetLayoutBinding        descriptorSetLayoutBindings[MAX_DESCRIPTOR];
    VkDescriptorPoolSize                descriptorPoolSize[MAX_DESCRIPTOR];
    VkWriteDescriptorSet                writeDescriptorSet[MAX_DESCRIPTOR];

    //VkDescriptorSetLayout               descriptorSetLayout;
    //VkDescriptorSet                     descriptorSet;
    //VkDescriptorPool                    descriptorPool;

    VkDescriptorSetAllocateInfo         descriptorSetAllocateInfo;

    vkcl_context                        *ctx;
    uint32_t                            bindings;
} vkcl_descset;

typedef struct{
    uint32_t                            bind_id;
    VkBufferCreateInfo                  info;
    VkBuffer                            buffer;

    VkDescriptorSetLayoutBinding        binding;

    VkDescriptorPoolSize                pool_size;
    VkDescriptorBufferInfo              desc_info;
    VkWriteDescriptorSet                update_info;
    vkcl_context                        *ctx;
    vkcl_descset                        *set;
    vkcl_memory                         *mem;
} vkcl_buffer;

typedef struct{
    VkDescriptorSetLayoutBinding        binding;
    VkImageCreateInfo                   info;
    VkImageView                         view;
    VkSampler                           sampler;
    VkImageLayout                       layout;
    VkImage                             image;
    VkMemoryRequirements                memReqs;

    VkDescriptorPoolSize                pool_size;
    VkDescriptorImageInfo               desc_info;
    VkWriteDescriptorSet                update_info;
    vkcl_context                        *ctx;
    vkcl_descset                        *set;
    vkcl_memory                         *mem;
} vkcl_image;

#define MAX_SETS                        4
typedef struct{
    VkShaderModule                      shader_module;
    uint32_t                            sets;
    VkDescriptorSetLayout               descriptorSetLayouts[MAX_SETS];
    VkDescriptorSet                     descriptorSets[MAX_SETS];
    VkDescriptorPool                    descriptorPools[MAX_SETS];
    VkPipelineLayout                    pipelineLayout;
    VkPipeline                          pipeline;

    VkCommandPool                       commandPool;
    VkCommandBuffer                     commandBuffer;
    vkcl_context                        *ctx;
} vkcl_pipeline;

#define VK_CHK(result) {  if (VK_SUCCESS != (result)) { fprintf(stderr, "Failure at %u %s\n", __LINE__, __FILE__); exit(-1); }  }

#define VKCL_GET_MEM(x) (x->mem)

void vkcl_init(vkcl_context *ctx);
void vkcl_end(vkcl_context *ctx);

vkcl_memory *vkcl_memory_allocate(vkcl_context *ctx, uint32_t flags, VkDeviceSize size);
void vkcl_memory_free(vkcl_memory *mem);
void *vkcl_memory_map(vkcl_memory *mem);
void vkcl_memory_unmap(vkcl_memory *mem);

vkcl_descset* vkcl_descset_create(vkcl_context *ctx, uint32_t set_id);
void vkcl_descset_destroy(vkcl_descset *set);

vkcl_buffer *vkcl_buffer_create(vkcl_context *ctx, vkcl_descset *set, uint32_t size, vkcl_memory *mem, uint32_t offset);
void vkcl_buffer_destroy(vkcl_buffer *buf);

vkcl_pipeline* vkcl_pipeline_create(vkcl_context *ctx, vkcl_descset **sets, uint32_t num_set, char *spv_fname);
void vkcl_pipeline_destroy(vkcl_pipeline* pipeline);
void vkcl_pipeline_exec(vkcl_pipeline *pipeline, uint32_t x, uint32_t y, uint32_t z);

#endif
