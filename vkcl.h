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

typedef struct{
    uint32_t                            set_id;
    VkDescriptorSetLayoutCreateInfo     layout_info;
    VkDescriptorSetLayout               layout;

    VkDescriptorPool                    desc_pool;
    VkDescriptorSetAllocateInfo         alloc_info;
    VkDescriptorSet                     set;

    VkWriteDescriptorSet                update;
    uint32_t                            mem_count;
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
    VkImage                             image;

    VkDescriptorPoolSize                pool_size;
    VkDescriptorImageInfo               desc_info;
    VkWriteDescriptorSet                update_info;
    vkcl_descset                        *set;
    vkcl_memory                         *mem;
} vkcl_image;

typedef struct{
    VkShaderModuleCreateInfo            shader_info;
    VkShaderModule                      shader_module;

    VkPipelineLayoutCreateInfo          info;
    VkComputePipelineCreateInfo         comp_info;
    VkPipeline                          pipeline;
} vkcl_pipeline;

typedef struct{
    VkCommandPoolCreateInfo             pool_info;
    VkCommandPool                       pool;
    VkCommandBufferAllocateInfo         buf_info;
    VkCommandBuffer                     buf;
    VkCommandBufferBeginInfo            begin_info;
} vkcl_command;

#define VK_CHK(result) {  if (VK_SUCCESS != (result)) { fprintf(stderr, "Failure at %u %s\n", __LINE__, __FILE__); exit(-1); }  }

#endif
